from typing import List, Callable

class LazyArray:
    def __init__(self, values: List[int], fn: Callable[[int], int] = lambda x: x):
        self.values = values
        self.fn = fn

    def map(self, fn: Callable[[int], int]):
        return LazyArray(self.values, lambda i: fn(self.fn(i)))
    
    def indexOf(self, target: int):
        for idx, v in enumerate(self.values):
            if self.fn(v) == target:
                return idx
        return -1

import unittest

class TestLazyArray(unittest.TestCase):
    @staticmethod
    def plusN(n: int):
        return lambda x: x + n

    def test_map(self):
        arr = LazyArray([1, 2, 3, 4, 5])
        self.assertEqual(arr.map(lambda x: x * 2).indexOf(2), 0)
        self.assertEqual(arr.map(self.plusN(1)).map(self.plusN(2)).indexOf(7), 3)

    def test_duplicate_values(self):
        arr = LazyArray([1, 1, 2, 2, 3])
        self.assertEqual(arr.map(lambda x: x * 2).indexOf(2), 0)
        self.assertEqual(arr.map(self.plusN(1)).map(self.plusN(2)).indexOf(5), 2)

    def test_map_chain_with_variables(self):
        a = LazyArray([1, 2, 1, 4, 5])
        b = a.map(self.plusN(1))
        c = b.map(self.plusN(2))
        d = c.map(self.plusN(3))
        self.assertEqual(d.indexOf(7), 0)
        self.assertEqual(c.indexOf(7), 3)
        self.assertEqual(b.indexOf(2), 0)
        self.assertEqual(a.indexOf(2), 1)

    def test_no_side_effects(self):
        a = LazyArray([1, 2, 3, 4, 5])
        b = a.map(self.plusN(1))
        self.assertEqual(a.indexOf(2), 1)

    def test_not_found(self):
        a = LazyArray([1, 2, 3, 4, 5])
        self.assertEqual(a.map(self.plusN(1)).indexOf(7), -1)

    def test_lazy_map(self):
        calls = 0
        def plusN_spy(n):
            nonlocal calls
            def plusN(x):
                nonlocal calls
                calls += 1
                return x + n
            return plusN
        a = LazyArray([1, 2, 3, 4, 5])
        b = a.map(plusN_spy(1))
        c = b.map(plusN_spy(2))
        self.assertEqual(calls, 0)
        self.assertEqual(c.indexOf(7), 3)


if __name__ == '__main__':
    unittest.main()