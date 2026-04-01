#!/usr/bin/env python3
"""
Scrape a local HTML page and send it as an HTML email via the local Postfix relay.

Usage:
    uv run --with requests send_portfolio.py [--url URL] [--to ADDRESS]

Env vars:
    PORTFOLIO_HOST  host for the portfolio URL (default: localhost)
    PORTFOLIO_PORT  port for the portfolio URL (default: 8888)
"""

import argparse
import os
import smtplib
import socket
import sys
from datetime import datetime
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

import requests


SMTP_HOST = "localhost"
SMTP_PORT = 25
_URL_HOST = os.environ.get("PORTFOLIO_HOST", "localhost")
_URL_PORT = os.environ.get("PORTFOLIO_PORT", "8888")
DEFAULT_URL = f"http://{_URL_HOST}:{_URL_PORT}/smart-stocker/portfolio.html"
DEFAULT_TO = "z@hczhu.me"
DEFAULT_FROM = "portfolio-emailer@mail.hczhu.me"


def scrape(url: str) -> str:
    resp = requests.get(url, timeout=10)
    resp.raise_for_status()
    return resp.content.decode("utf-8")


def send(html: str, to: str, from_addr: str, url: str) -> None:
    subject = f"Daily portfolio snapshot - {datetime.now().strftime('%Y-%m-%d')}"

    msg = MIMEMultipart("alternative")
    msg["Subject"] = subject
    msg["From"] = from_addr
    msg["To"] = to
    msg.attach(MIMEText(html, "html", "utf-8"))

    with smtplib.SMTP(SMTP_HOST, SMTP_PORT) as smtp:
        refused = smtp.sendmail(from_addr, [to], msg.as_string())
    if refused:
        for addr, (code, err_msg) in refused.items():
            print(f"ERROR: recipient {addr} refused – {code} {err_msg.decode()}", file=sys.stderr)
        sys.exit(1)
    print(f"Email sent to {to} (subject: {subject})")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", default=DEFAULT_URL, help="Page to scrape")
    parser.add_argument("--to", default=DEFAULT_TO, help="Recipient email address")
    parser.add_argument("--from", dest="from_addr", default=DEFAULT_FROM,
                        help="Sender address")
    args = parser.parse_args()

    print(f"Scraping {args.url} …")
    html = scrape(args.url)
    print(f"Fetched {len(html)} bytes; sending email to {args.to} …")
    send(html, args.to, args.from_addr, args.url)


if __name__ == "__main__":
    main()
