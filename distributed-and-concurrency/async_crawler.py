
import asyncio
from collections import defaultdict
import http
from venv import create
from httpx import AsyncClient, HTTPStatusError
from queue import SimpleQueue
from typing import NamedTuple
from dataclasses import dataclass
from collections.abc import Iterator, AsyncIterator

@dataclass
class CrawResult:
    url: str
    domain: str
    content: bytes | None = None
    error: str | None = None

class CrawlOptions(NamedTuple):
    timeout_seconds: int = 10
    # Overall max concurrency
    max_pending: int = 10
    max_pending_per_domain: int = 1
    max_pending_per_domain_overrides: dict[str, int] = {}
    # TODO: rate limit per domain

@dataclass
class CrawlTask:
    url: str
    # Will be populated by this crawler if not polulated by the caller.
    domain: str | None = None


class AsyncCrawler:
    def __init__(self, opts: CrawlOptions) -> None:
        self.opts = opts
        self.pending_tasks: list[CrawlTask] = []
        self.pending_tasks_per_domain: defaultdict[str, int] = defaultdict(int)
        self.task_queue = SimpleQueue[CrawlTask]()
        # If a task is limited by max_pending_per_domain, it will be put in one of those queues.
        self.domain_queues: defaultdict[str, SimpleQueue[CrawlTask]] = defaultdict(SimpleQueue)

    async def crawl_one(self, task: CrawlTask, client: AsyncClient) -> CrawResult:
        resp = await client.get(task.url, timeout=self.opts.timeout_seconds, follow_redirects=True)
        result = CrawResult(
            url=task.url,
            domain=task.domain if task.domain is not None else "",
        )
        try:
            resp.raise_for_status()
        except HTTPStatusError as e:
            result.error = str(e)
        else:
            result.content = resp.content
        return result
    
    async def crawl_all(self, tasks: Iterator[CrawlTask]) -> AsyncIterator[CrawResult]:
        async with AsyncClient() as client:
            async for result in self.crawl_all_with_client(tasks, client):
                yield result

    async def crawl_all_with_client(self, tasks: Iterator[CrawlTask], client: AsyncClient) -> AsyncIterator[CrawResult]:
        for task in tasks:
            yield await self.crawl_one(task, client)

