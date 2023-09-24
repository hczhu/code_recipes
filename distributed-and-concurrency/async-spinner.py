#!/usr/bin/env python3.10
# -*- coding: utf-8 -*-

import itertools
import time
import logging
import threading
import multiprocessing as mp
from typing import Any, TypeVar, Protocol, Type
import typing

class Event(Protocol):
    e = threading.Event()
    e.wait
    def wait(self, timeout: float | None) -> bool:
        pass
    def set(self) -> None:
        pass

class Thread(Protocol):
    def __init__(self) -> None:
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

def supervisor(event_type: typing.Type[TEvent], thread_type: typing.Type[TThread]):
    done = event_type()
    spinner = thread_type(target=spin, args=("thinking!", done), name="spinner")
    logging.info(f"spinner object:{spinner}")
    spinner.start()
    result = slow()
    logging.info("Got result from slow()")
    done.set()
    logging.info("Joining spinner thread")
    spinner.join()
    return result

def main():
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s p:%(processName)s t:%(threadName)s %(filename)s:%(lineno)s %(levelname)s: %(message)s"
    )
    print(f"Answer: {supervisor(threading.Event, threading.Thread)}")
    print(f"Answer: {supervisor(mp.Event, mp.Process)}")

if __name__ == "__main__":
    main()
