#!/usr/bin/env python3
"""
Scrape the Minecraft Bedrock Server download page using a headless browser
to find the latest Linux server download URL.

Strategy:
  1. Intercept all network responses to catch any API call that returns the
     download URL (the page fetches it dynamically via JS).
  2. Fall back to DOM scraping if a direct link appears in the rendered HTML.
"""

import re
import sys
import json
from playwright.sync_api import sync_playwright, TimeoutError as PlaywrightTimeout

URL = "https://www.minecraft.net/en-us/download/server/bedrock"
ZIP_RE = re.compile(r'https://[^\s"\'<>]+bin-linux[^\s"\'<>]+\.zip')


def try_browser(p, browser_type_name: str) -> str:
    browser_type = getattr(p, browser_type_name)
    launch_kwargs = dict(headless=True)
    if browser_type_name == "chromium":
        launch_kwargs["args"] = [
            "--disable-http2",
            "--disable-blink-features=AutomationControlled",
            "--no-sandbox",
        ]

    print(f"Trying {browser_type_name}...", file=sys.stderr)
    browser = browser_type.launch(**launch_kwargs)
    context = browser.new_context(
        user_agent=(
            "Mozilla/5.0 (X11; Linux x86_64) "
            "AppleWebKit/537.36 (KHTML, like Gecko) "
            "Chrome/124.0.0.0 Safari/537.36"
        ),
        locale="en-US",
        extra_http_headers={
            "Accept-Language": "en-US,en;q=0.9",
        },
    )
    if browser_type_name == "chromium":
        context.add_init_script(
            "Object.defineProperty(navigator, 'webdriver', {get: () => undefined})"
        )

    found_url = []

    def on_response(response):
        if found_url:
            return
        try:
            ct = response.headers.get("content-type", "")
            if "json" in ct or "javascript" in ct or "text" in ct:
                body = response.text()
                m = ZIP_RE.search(body)
                if m:
                    print(
                        f"  Found URL in network response: {response.url}",
                        file=sys.stderr,
                    )
                    found_url.append(m.group(0))
        except Exception:
            pass

    page = context.new_page()
    page.on("response", on_response)

    try:
        page.goto(URL, wait_until="commit", timeout=60_000)
    except Exception as e:
        print(f"  goto error: {e}", file=sys.stderr)
        browser.close()
        return ""

    # Give JS time to run and fetch data
    if not found_url:
        try:
            page.wait_for_selector(
                "a[href*='bin-linux'][href$='.zip']", timeout=20_000
            )
        except PlaywrightTimeout:
            pass

    # Check DOM
    if not found_url:
        dom_url = page.get_attribute(
            "a[href*='bin-linux'][href$='.zip']", "href"
        )
        if dom_url:
            found_url.append(dom_url)

    # Scan full rendered HTML as last resort
    if not found_url:
        html = page.content()
        m = ZIP_RE.search(html)
        if m:
            found_url.append(m.group(0))

    if not found_url:
        print(f"  {browser_type_name}: no URL found.", file=sys.stderr)
        print(f"  Page title: {page.title()}", file=sys.stderr)

    browser.close()
    return found_url[0] if found_url else ""


def get_linux_download_url() -> str:
    with sync_playwright() as p:
        for browser_name in ("chromium", "firefox", ):
            try:
                url = try_browser(p, browser_name)
                if url:
                    return url
            except Exception as e:
                print(f"{browser_name} failed: {e}", file=sys.stderr)
    return ""


if __name__ == "__main__":
    download_url = get_linux_download_url()
    if download_url:
        print(download_url)
    else:
        print("ERROR: Could not find Linux download URL.", file=sys.stderr)
        sys.exit(1)
