# https://docs.python.org/3/library/threading.html
from concurrent.futures import thread
import threading
class Foo:
    def __init__(self):
        self.b1 = threading.Barrier(2)
        self.b2 = threading.Barrier(2)


    def first(self, printFirst: 'Callable[[], None]') -> None:
        
        # printFirst() outputs "first". Do not change or remove this line.
        printFirst()

        self.b1.wait()


    def second(self, printSecond: 'Callable[[], None]') -> None:
        
        self.b1.wait()
        # printSecond() outputs "second". Do not change or remove this line.
        printSecond()
        self.b2.wait()


    def third(self, printThird: 'Callable[[], None]') -> None:
        
        self.b2.wait()
        # printThird() outputs "third". Do not change or remove this line.
        printThird()

