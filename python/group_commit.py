from collections.abc import Callable
import threading
import time
from typing import TypeVar, Generic
import logging
import random

T = TypeVar("T")

class CommitEvent(Generic[T]):
    def __init__(self, data: T):
        self.data = data
        self.event = threading.Event()
    
    def get_event(self) -> threading.Event:
        return self.event

    def get_data(self) -> T:
        return self.data
    
def init_logger(level: int) -> logging.Logger:
    logger = logging.getLogger(__name__)
    handler = logging.StreamHandler()
    formatter = logging.Formatter(
        "%(asctime)s p:%(processName)s t:%(threadName)s %(filename)s:%(lineno)s %(levelname)s: %(message)s")
    handler.setFormatter(formatter)
    logger.addHandler(handler)
    logger.setLevel(level)
    return logger

class GroupCommit(Generic[T]):
    """
    Implememnt https://www.burnison.ca/notes/fun-mysql-fact-of-the-day-group-commit
    """ 
    def __init__(
        self,
        max_concurrent_commits: int,
        merge_func: Callable[[T, T], T],
        commit_func: Callable[[T], None],
        logger: logging.Logger|None = None
    ):
        self.commit_sem = threading.Semaphore(max_concurrent_commits)
        self.merge_func = merge_func
        self.commit_func = commit_func
        self.queue_lock = threading.Lock()
        self.queue: list[CommitEvent[T]] = []
        self.logger = logger if logger is not None else init_logger(logging.INFO)
    
    def commit(self, data: T) -> int:
        ce = CommitEvent[T](data)
        with self.queue_lock:
            self.logger.info(f"Process {threading.current_thread().name} got queue lock")
            is_leader = len(self.queue) == 0
            self.queue.append(ce)
        
        if not is_leader:
            ce.get_event().wait()
            return 0

        # Give some time for other threads to put their data in the queue
        time.sleep(random.random() * 0.001)
        with self.queue_lock:
            q = self.queue
            self.queue = []

        commits = len(q)
        merged = q[0].get_data()
        for d in q[1:]:
            merged = self.merge_func(merged, d.get_data())
        self.logger.info(f"Merged {len(q)} commits and waiting for committing semaphore ...")
        with self.commit_sem:
            self.logger.info(f"Committing now ...")
            self.commit_func(merged)
        self.logger.info(f"Commited {len(q)} commits as a group")
        for d in q[1:]:
            d.get_event().set()
        q = []
        return commits

if __name__ == "__main__": 
    commits = []
    returns = []
    lock = threading.Lock()
    logger = init_logger(logging.INFO)
    
    def merge_func(a: int, b: int) -> int:
        return a + b
    
    def commit_func(v: int):
        logger.info(f"Committing {v} ...")
        time.sleep(random.random() * 0.1)
        with lock:
            commits.append(v)

    gc = GroupCommit[int](
        max_concurrent_commits=2,
        merge_func=merge_func,
        commit_func=commit_func,
        logger=logger
    )
    
    def run_commit(v: int):
        logger.info(f"Thread {threading.current_thread().name} committing {v}")
        res = gc.commit(v)
        with lock:
            returns.append(res)

    logger.info("Starting main thread ...")
    committers = []
    s = 0
    num_threds = 1000
    for i in range(1000):
        d = i + 1
        s += d
        committers.append(
            threading.Thread(
                target=run_commit,
                args=(d,),
                name=f"committer-{i}"
            )
        )
    for c in committers:
        c.start()

    for c in committers:
        c.join()

    returns = [r for r in returns if r > 0]
    print(f"run_commits returns: {list(returns)}")
    assert len(returns) == len(commits)
    assert sum(returns) == num_threds

    print(f"{len(commits)} commits: {list(commits)}")
    assert sum(commits) == s

