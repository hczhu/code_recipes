
import asyncio
from collections import defaultdict
import time
from httpx import AsyncClient, HTTPStatusError, TimeoutException
from queue import SimpleQueue
from typing import NamedTuple
from dataclasses import dataclass, field
from collections.abc import Iterator, AsyncIterator
from typing import Callable
import logging
import tldextract
import sys

@dataclass
class CrawResult:
    url: str
    domain: str
    content: bytes | None = None
    headers: dict[str, str] = field(default_factory=dict)
    error: str | None = None
    # How long did it take to crawl this URL?
    walltime_seconds: float = 0
    def __str__(self) -> str:
        return (f"CrawResult(url={self.url}, domain={self.domain}, walltime ms={int(1000 * self.walltime_seconds)}, " +
            f"content encoding={self.headers.get('content-encoding', '')}, "
            f"content length in header={self.headers.get('content-length', '-1')}, "
            f"content length={len(self.content) if self.content is not None else 0}, error={self.error}, )"
            f"content={self.content[:100] if self.content is not None else ''}"
        )

class CrawlOptions(NamedTuple):
    headers: dict[str, str] = {}
    timeout_seconds: int = 3
    # Overall max concurrency
    max_pending: int = 3
    max_pending_per_domain: int = 1
    max_pending_per_domain_overrides: dict[str, int] = {}
    # NB: don't block the even loop used by the crawler. Offload the processing to anohther thread/process.
    post_processor: Callable[[CrawResult], None] = lambda result: None
    # TODO: rate limit per domain

@dataclass
class CrawlTask:
    url: str
    # Will be populated by this crawler if not polulated by the caller.
    domain: str | None = None


class AsyncCrawler:
    def __init__(self, opts: CrawlOptions, logger: logging.Logger|None = None) -> None:
        self.opts = opts
        self.pending_tasks: list[asyncio.Task] = []
        self.pending_tasks_per_domain: defaultdict[str, int] = defaultdict(int)
        self.task_queue = SimpleQueue[CrawlTask]()
        # If a task is limited by max_pending_per_domain, it will be put in one of those queues.
        self.domain_queues: defaultdict[str, SimpleQueue[CrawlTask]] = defaultdict(SimpleQueue)
        self.headers = opts.headers
        if logger is None:
            logger = logging.getLogger(__name__)
            handler = logging.StreamHandler()
            formatter = logging.Formatter(
                f"%(asctime)s p:%(processName)s t:%(threadName)s %(filename)s:%(lineno)s %(levelname)s: %(message)s")
            handler.setFormatter(formatter)
            logger.addHandler(handler)
            logger.setLevel(logging.DEBUG)
        self.logger = logger

    async def crawl_one(self, task: CrawlTask, client: AsyncClient) -> CrawResult:
        self.logger.info(f"Crawling {task.url}")
        result = CrawResult(
            url=task.url,
            domain=task.domain if task.domain is not None else "",
        )
        try:
            result.walltime_seconds = time.time()
            # HTTPX automatically decompresses the returned content. It can handle responses sent with gzip or deflate compression.
            resp = await client.get(
                task.url,
                timeout=self.opts.timeout_seconds,
                follow_redirects=True,
                headers=self.headers,
            )
            result.walltime_seconds = time.time() - result.walltime_seconds
            resp.raise_for_status()
            result.headers = dict(resp.headers)
        except HTTPStatusError as e:
            self.logger.info(f"Got an HTTPStatusError from {task.url}: {e}")
            result.error = f"HTTPStatusError: {e}"
        except TimeoutException as e:
            self.logger.info(f"Got a timeout from {task.url}: {e}")
            result.error = f"TimeoutException after {self.opts.timeout_seconds} seconds: {e}"
        except Exception as e:
            self.logger.info(f"Got an exception from {task.url}: {e}")
            result.error = f"Unknown exception: {e}"
        else:
            result.content = resp.content
        return result
    
    async def crawl_all(self, tasks: Iterator[CrawlTask]) -> None:
        async with AsyncClient() as client:
            async for result in self.crawl_all_with_client(tasks, client):
                self.opts.post_processor(result)

    def populate_pending_tasks(self, tasks: Iterator[CrawlTask], client: AsyncClient) -> None:
        if len(self.pending_tasks) >= self.opts.max_pending:
            return
        for task in tasks:
            if task.domain is None:
                parsed = tldextract.extract(task.url)
                task.domain = parsed.domain + "." + parsed.suffix
            assert self.pending_tasks_per_domain[task.domain] <= self.opts.max_pending_per_domain
            if self.pending_tasks_per_domain[task.domain] >= self.opts.max_pending_per_domain:
                self.logger.info(f"Delayed crawling {task.url} because we already have {self.pending_tasks_per_domain[task.domain]}"
                                 f" pending tasks for domain {task.domain}")
                self.domain_queues[task.domain].put(task)
            else:
                self.pending_tasks.append(asyncio.create_task(self.crawl_one(task, client)))
                self.pending_tasks_per_domain[task.domain] += 1
            if len(self.pending_tasks) == self.opts.max_pending:
                return

    async def crawl_all_with_client(self, tasks: Iterator[CrawlTask], client: AsyncClient) -> AsyncIterator[CrawResult]:
        while True:
            self.populate_pending_tasks(tasks, client)
            self.logger.info(f"Got {len(self.pending_tasks)} pending tasks")
            done, pending_tasks = await asyncio.wait(self.pending_tasks, return_when=asyncio.FIRST_COMPLETED)
            self.pending_tasks = list(pending_tasks)
            for ft in done:
                result = await ft
                self.pending_tasks_per_domain[result.domain] -= 1
                if not self.domain_queues[result.domain].empty():
                    self.logger.info(f"Found a pending task for domain {result.domain}")
                    self.pending_tasks.append(asyncio.create_task(self.crawl_one(self.domain_queues[result.domain].get(), client)))
                    self.pending_tasks_per_domain[result.domain] += 1
                yield result
            if len(self.pending_tasks) == 0:
                break

if __name__ == '__main__':
    max_pending = 3
    if len(sys.argv) > 1:
        max_pending = int(sys.argv[1])
    crawler = AsyncCrawler(CrawlOptions(
        max_pending=max_pending,
        post_processor=lambda result: print_result(result),
    ))
    crawler.logger.info(f"Crawler options: {crawler.opts}")
    def show(result: CrawResult) -> None:
        crawler.logger.info(f"Got result {result}")
        print(result)

    def print_result(result: CrawResult) -> None:
        loop = asyncio.get_running_loop()
        loop.run_in_executor(None, lambda r: show(r), result)

    def gen_urls() -> Iterator[CrawlTask]:
        for line in sys.stdin:
            yield CrawlTask(url=line.strip())

    asyncio.run(crawler.crawl_all(gen_urls()))