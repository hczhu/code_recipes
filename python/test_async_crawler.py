pytest_plugins = ['pytest_asyncio']

""" Run the tests with: pytest test_async_crawler.py -n auto"""

import asyncio
import time
import pytest
import logging
from httpx import AsyncClient
from async_crawler import AsyncCrawler, CrawlOptions, CrawlTask, CrawResult

# A fake response to simulate httpx.AsyncClient.get behavior.
class FakeResponse:
    def __init__(self):
        self.headers = {"content-encoding": "utf-8", "content-length": "11"}
        self.content = b"hello world"
    def raise_for_status(self):
        pass

# A fake AsyncClient to inject the fake get method.
class FakeAsyncClient:
    def __init__(self, delaySeconds=0):
        self.delaySeconds = delaySeconds
    async def get(self, url, timeout, follow_redirects, headers):
        await asyncio.sleep(self.delaySeconds)  # simulate async call
        return FakeResponse()
    async def __aenter__(self):
        return self
    async def __aexit__(self, exc_type, exc, tb):
        pass

@pytest.mark.asyncio
async def test_crawl_one_success(test_logger):
    # Setup crawler with dummy options and logger.
    opts = CrawlOptions(
        headers={},
        timeout_seconds=3,
        max_pending=3,
        max_pending_per_domain=1,
        max_pending_per_domain_overrides={},
        post_processor=lambda result: None
    )
    crawler = AsyncCrawler(opts, test_logger)
    task = CrawlTask(url="https://example_crawl_one.com")
    
    # Use FakeAsyncClient instead of httpx.AsyncClient.
    fake_client = FakeAsyncClient()
    result = await crawler.crawl_one(task, fake_client)
    # Check the result
    assert result.content == b"hello world"
    assert result.error is None

@pytest.mark.asyncio
async def test_craw_all_1_1_0(test_logger):
   await run_craw_all(max_pending=1, max_pending_per_domain=1, slow=False, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_2_1_0(test_logger):
   await run_craw_all(max_pending=2, max_pending_per_domain=1, slow=False, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_2_2_0(test_logger):
   await run_craw_all(max_pending=2, max_pending_per_domain=2, slow=False, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_3_1_0(test_logger):
   await run_craw_all(max_pending=3, max_pending_per_domain=1, slow=False, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_3_2_0(test_logger):
   await run_craw_all(max_pending=3, max_pending_per_domain=2, slow=False, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_3_3_0(test_logger):
   await run_craw_all(max_pending=3, max_pending_per_domain=3, slow=False, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_1_1_1(test_logger):
   await run_craw_all(max_pending=1, max_pending_per_domain=1, slow=True, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_2_1_1(test_logger):
   await run_craw_all(max_pending=2, max_pending_per_domain=1, slow=True, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_2_2_1(test_logger):
   await run_craw_all(max_pending=2, max_pending_per_domain=2, slow=True, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_3_1_1(test_logger):
   await run_craw_all(max_pending=3, max_pending_per_domain=1, slow=True, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_3_2_1(test_logger):
   await run_craw_all(max_pending=3, max_pending_per_domain=2, slow=True, logger=test_logger)

@pytest.mark.asyncio
async def test_craw_all_3_3_1(test_logger):
   await run_craw_all(max_pending=3, max_pending_per_domain=3, slow=True, logger=test_logger)

async def run_craw_all(max_pending=3, max_pending_per_domain=1, slow=False, logger=None):
    # Define a generator of tasks.
    def gen_tasks():
        yield CrawlTask(url="https://example.com/1")
        yield CrawlTask(url="https://example.com/2")
        yield CrawlTask(url="https://example.org/3")
        yield CrawlTask(url="https://example.org/4")
    
    # Create a dummy post_processor with local results list
    results = []
    def post_processor(result: CrawResult):
        results.append(result)
    
    opts = CrawlOptions(
        headers={},
        timeout_seconds=3,
        max_pending=max_pending,
        max_pending_per_domain=max_pending_per_domain,
        max_pending_per_domain_overrides={},
        post_processor=post_processor
    )
    logger = logging.getLogger(f"test_crawler_{max_pending}_{max_pending_per_domain}_{slow}")
    crawler = AsyncCrawler(opts, logger)
    
    async with FakeAsyncClient(1 if slow else 0) as client:
        async for result in crawler.crawl_all_with_client(gen_tasks(), client):
            pass
    
    # Check the results
    assert len(results) == 4, f"{results}"
    for res in results:
        assert res.content == b"hello world"
        assert res.error is None
        assert res.headers == {"content-encoding": "utf-8", "content-length": "11"}
        assert isinstance(res.walltime_seconds, float)
    return results  # Return results for any additional checks if needed