#!/usr/bin/env python3.10
# -*- coding: utf-8 -*-

import asyncio
import itertools
import time
import logging
import threading
import multiprocessing as mp
from typing import Any, TypeVar, Protocol, Type, Callable
import typing

class Event(Protocol):
    def wait(self, timeout: float | None) -> bool:
        return True
    def set(self) -> None:
        pass

class Thread(Protocol):
    def __init__(self, group: Any=None, target: Any=None, name: Any=None, args: Any=None) -> None:
        pass
    def start(self) -> None:
        pass
    def join(self) -> None:
        pass

TEvent = TypeVar("TEvent", bound=Event)
TThread = TypeVar("TThread", bound=Thread)

def slow() -> int:
    """A slow function."""
    time.sleep(3)
    return 42

def spin(msg: str, done: TEvent):
    """Prints a message while waiting for another thread to finish."""
    logging.info("spin started")
    status = ""
    for char in itertools.cycle('|/-\\'):
        status = f"\r{char} {msg}"
        print(status, end="", flush=True)
        if done.wait(.1):
            break
    blanks = ' ' * len(status)
    print(f"\r{blanks}\r", end="", flush=True)
    logging.info("spin ended")

def supervisor(event_factor: Callable[[], TEvent], thread_type: typing.Type[TThread]):
    done = event_factor()
    spinner = thread_type(target=spin, args=("thinking!", done), name="spinner")
    logging.info(f"spinner object:{spinner}")
    spinner.start()
    result = slow()
    logging.info("Got result from slow()")
    done.set()
    logging.info("Joining spinner thread")
    spinner.join()
    return result

async def coro_spin(msg: str):
    """Prints a message while waiting for another thread to finish."""
    logging.info("spin started")
    status = ""
    for char in itertools.cycle('|/-\\'):
        status = f"\r{char} {msg}"
        print(status, end="", flush=True)
        try:
            await asyncio.sleep(.1)
        except asyncio.CancelledError:
            break
    blanks = ' ' * len(status)
    print(f"\r{blanks}\r", end="", flush=True)
    logging.info("spin ended")

async def coro_slow() -> int:
    """A slow function."""
    await asyncio.sleep(3)
    return 42

async def coro_supervisor() -> int:
    spinner = asyncio.create_task(coro_spin("thinking!"))
    logging.info(f"spinner object:{spinner}")
    result = await coro_slow()
    spinner.cancel()
    return result

def main():
    logging.basicConfig(
        level=logging.INFO,
        format="p:%(processName)s t:%(threadName)s %(filename)s:%(lineno)s %(levelname)s: %(message)s"
    )
    start = time.perf_counter()
    print(f"Answer: {supervisor(threading.Event, threading.Thread)} after {time.perf_counter() - start:.3f} seconds")
    start = time.perf_counter()
    print(f"Answer: {supervisor(mp.Event, mp.Process)} after {time.perf_counter() - start:.3f} seconds")
    start = time.perf_counter()
    print(f"Answer: {asyncio.run(coro_supervisor())} after {time.perf_counter() - start:.3f} seconds")

if __name__ == "__main__":
    main()
