from typing import List, Callable

class LazyArray:
    def __init__(self, values: List[int]):
        self.values = values
        self.fn: callable[[int], int] = lambda x: x

    
    def map(self, fn: Callable[[int], int]):
        nested = LazyArray(self.values)
        nested.fn = lambda x: fn(self.fn(x))
    
    def indexOf(self, needle: int):
        for idx, v in enumerate(self.values):
            if self.fn(v) == needle:
                return idx
        return -1

import unittest

class TestLazyArray(unittest.TestCase):
    def test_map(self):
        arr = LazyArray([1, 2, 3, 4, 5])
        arr.map(lambda x: x * 2)
        self.assertEqual(arr.values, [2, 4, 6, 8, 10])

if __name__ == '__main__':
    unittest.main()