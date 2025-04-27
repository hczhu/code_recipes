import asyncio
from collections import defaultdict
import time
from httpx import AsyncClient, HTTPStatusError, TimeoutException
from queue import SimpleQueue
from typing import NamedTuple, Optional
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
    content: Optional[bytes] = None
    headers: dict[str, str] = field(default_factory=dict)
    error: Optional[str] = None
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
    domain: Optional[str] = None


class AsyncCrawler:
    def __init__(self, opts: CrawlOptions, logger: logging.Logger|None = None) -> None:
        self.opts = opts
        self.pending_tasks: list[asyncio.Task] = []
        self.pending_tasks_per_domain: defaultdict[str, int] = defaultdict(int)
        # If a task is limited by max_pending_per_domain, it will be put in one of those queues.
        self.domain_queues: defaultdict[str, SimpleQueue[CrawlTask]] = defaultdict(SimpleQueue)
        self.headers = opts.headers
        if logger is None:
            logger = logging.getLogger(__name__)
            # Only add handler if none exists
            if not logger.handlers:
                handler = logging.StreamHandler()
                formatter = logging.Formatter(
                    f"%(asctime)s p:%(processName)s t:%(threadName)s %(filename)s:%(lineno)s %(levelname)s: %(message)s")
                handler.setFormatter(formatter)
                logger.addHandler(handler)
                logger.setLevel(logging.DEBUG)
        self.logger = logger

    async def crawl_one(self, task: CrawlTask, client: AsyncClient) -> CrawResult:
        self.logger.info(f"Crawling {task}")
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
    
    async def crawl_all(self, tasks: Iterator[CrawlTask]) -> AsyncIterator[CrawResult]:
        self.task_iter = tasks
        self.waiting_tasks = 0
        self.no_more_tasks = False
        async with AsyncClient() as client:
            async for result in  self.crawl_all_with_client(tasks, client):
                yield result

    def populate_pending_tasks(self, client: AsyncClient) -> None:
        def get_nex_task() -> Optional[CrawlTask]:
            try:
                return next(self.task_iter)
            except StopIteration:
                self.no_more_tasks = True
                return None
            return None

        while len(self.pending_tasks) < self.opts.max_pending:
            task = get_nex_task()
            if task is None:
                self.no_more_tasks = True
                break
            if task.domain is None:
                parsed = tldextract.extract(task.url)
                task.domain = parsed.domain + "." + parsed.suffix
            assert self.pending_tasks_per_domain[task.domain] <= self.opts.max_pending_per_domain
            if self.pending_tasks_per_domain[task.domain] >= self.opts.max_pending_per_domain:
                self.logger.info(f"Delayed crawling {task.url} because we already have {self.pending_tasks_per_domain[task.domain]}"
                                 f" pending tasks for domain {task.domain}")
                self.domain_queues[task.domain].put(task)
                self.waiting_tasks += 1
            else:
                self.pending_tasks.append(asyncio.create_task(self.crawl_one(task, client)))
                self.pending_tasks_per_domain[task.domain] += 1

    async def crawl_all_with_client(self, tasks: Iterator[CrawlTask], client: AsyncClient) -> AsyncIterator[CrawResult]:
        self.task_iter = tasks
        self.waiting_tasks = 0
        self.no_more_tasks = False
        self.logger.info(f"Starting crawl with {self.opts.max_pending} max pending tasks")
        while True:
            self.populate_pending_tasks(client)
            if len(self.pending_tasks) == 0:
                break
            self.logger.debug(f"Got {len(self.pending_tasks)} pending tasks")
            done, pending_tasks = await asyncio.wait(self.pending_tasks, return_when=asyncio.FIRST_COMPLETED)
            self.logger.debug(f"Got {len(done)} done tasks")
            self.pending_tasks = list(pending_tasks)
            for ft in done:
                result = ft.result()
                self.logger.info(f"Got result {result}")
                self.pending_tasks_per_domain[result.domain] -= 1
                # TODO: this domain will keep its quota until all tasks in this domain queue are done. It may starve other domains.
                if not self.domain_queues[result.domain].empty():
                     self.logger.info(f"Found a pending task for domain {result.domain}")
                     self.pending_tasks.append(asyncio.create_task(self.crawl_one(self.domain_queues[result.domain].get(), client)))
                     self.pending_tasks_per_domain[result.domain] += 1
                     self.waiting_tasks -= 1
                self.opts.post_processor(result)
                yield result

if __name__ == '__main__':
    max_pending = 2
    if len(sys.argv) > 1:
        max_pending = int(sys.argv[1])
    crawler = AsyncCrawler(CrawlOptions(
        max_pending=max_pending,
        max_pending_per_domain=1,
        post_processor=lambda result: print_result(result),
    ))
    results = []
    crawler.logger.info(f"Crawler options: {crawler.opts}")
    
    def show(result: CrawResult) -> None:
        crawler.logger.info(f"Got result {result}")
        print(result)
    
    def print_result(result: CrawResult) -> None:
        results.append(result)
        loop = asyncio.get_running_loop()
        loop.run_in_executor(None, lambda r: show(r), result)
    
    def gen_urls() -> Iterator[CrawlTask]:
        yield CrawlTask(url="https://api.tickertick.com/feed")
        yield CrawlTask(url="https://api.tickertick.com/tickers")
        yield CrawlTask(url="https://www.google.com")
    
    async def main():
        async for _ in crawler.crawl_all(gen_urls()):
            pass

    asyncio.run(main())

    assert len(results) == 3, f"{results}"