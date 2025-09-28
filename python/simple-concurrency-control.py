# https://docs.python.org/3/library/threading.html
from concurrent.futures import thread
from typing import Callable
import threading
def printFirst():
    print("first", end=' ')
def printSecond():
    print("second", end=' ')
def printThird():
    print("third", end=' ')
class Foo:
    def __init__(self):
        self.b1 = threading.Barrier(2)
        self.b2 = threading.Barrier(2)


    def first(self, printFirst: Callable[[], None]) -> None:
        
        # printFirst() outputs "first". Do not change or remove this line.
        printFirst()

        self.b1.wait()


    def second(self, printSecond: Callable[[], None]) -> None:
        
        self.b1.wait()
        # printSecond() outputs "second". Do not change or remove this line.
        printSecond()
        self.b2.wait()


    def third(self, printThird: Callable[[], None]) -> None:
        
        self.b2.wait()
        # printThird() outputs "third". Do not change or remove this line.
        printThird()

if __name__ == "__main__":
    foo = Foo()
    t1 = threading.Thread(target=foo.first, args=(printFirst,))
    t2 = threading.Thread(target=foo.second, args=(printSecond,))
    t3 = threading.Thread(target=foo.third, args=(printThird,))
    t3.start()
    t2.start()
    t1.start()
    t1.join()
    t2.join()
    t3.join()