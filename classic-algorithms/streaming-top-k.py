#!/usr/bin/env python3

import sys
import typing
import random
from typing import TypeVar, List, Generator
T = TypeVar('T')

# Return the k-th smallest element in arr. k is 0-based.
def quick_select(arr: List[T], k) -> T:
    l, r = 0, len(arr)
    assert 0 <= k and k < r - l, (l, r, k)
    while l + 1 < r:
        # An invariant
        assert 0 <= k and k < r - l, (l, r, k)
        pivot = arr[(l + r) // 2]
        i, j = l, r - 1
        while i < j:
            # i and j won't run out of bounds because of the pivot value.
            while arr[i] < pivot:
                i += 1
            while arr[j] > pivot:
                j -= 1
            if i < j:
                arr[i], arr[j] = arr[j], arr[i]
                i += 1
                j -= 1
        # Invariants:
        #  1. l <= j < r - 1 (j must have moved because the index of the pivot < j)
        #  2. l <= i <= r - 1 (i may not have moved because the index of the pivot >= i)
        #  3. arr[l:i] <= pivot and pivot <= arr[j+1:r].
        #  4. i >= j
        # What if i == j? Then arr[l:i] <= pivot and pivot <= arr[i+1:r]. arr[i] is undertermined.
        if i == j and arr[i] < pivot:
            i += 1
        if k < i - l:
            r = i
        else:
            k -= i - l
            l = i
    return arr[l]

def streaming_top_k(input_yielder: Generator[T, None, None], k) -> List[T]:
    """Returns the top k (1-based) elements from the input stream.
    It works well for relatively small k and large input stream.
    """
    def append_from_yielder(yielder, arr, n):
        for v in yielder:
            arr.append(v)
            n -= 1
            if n == 0:
                break
    arr = []
    append_from_yielder(input_yielder, arr, 2 * k)
    while len(arr) > k:
        k_largest = quick_select(arr, len(arr) - k)
        l = 0
        for v in arr:
            if v > k_largest:
                arr[l] = v
                l += 1
                if l == k:
                    break
        arr = arr[0:l] + (k - l) * [k_largest]
        append_from_yielder(input_yielder, arr, k)
    assert len(arr) == k, (len(arr), k)
    return arr

import unittest

class Test(unittest.TestCase):
    def test_quick_select(self):
        self.assertEqual(quick_select([1, 2, 3, 4, 5], 0), 1)
        self.assertEqual(quick_select([1, 2, 3, 4, 5], 4), 5)
        self.assertEqual(quick_select([1, 2, 3, 4, 5], 2), 3)
        self.assertEqual(quick_select([0 for _ in range(1000000)], 100000), 0)

    def test_small_ints(self):
        def gen():
            return iter([1, 2, 3, 4, 5])
        self.assertEqual(sorted(streaming_top_k(gen(), 3)), sorted([3, 4, 5]))

    def test_small_str(self):
        s = "hello world"
        def gen():
            for c in s:
                yield c
        l = [' ', 'e', 'h', 'l', 'l', 'o', 'l', 'o', 'r', 'w']
        self.assertEqual(quick_select(l, 5), sorted(l)[5])
        self.assertEqual(quick_select(list(s), len(s) - 5), 'l')
        self.assertEqual(sorted(streaming_top_k(gen(), 5)), sorted(s)[-5:])

    def test_large(self):
        def gen():
            ints = list(range(100000))
            random.shuffle(ints)
            return iter(ints)
        self.assertEqual(sorted(streaming_top_k(gen(), 501)), sorted(list(range(99499, 100000))))

    def test_degen(self):
        def gen():
            for _ in range(1000000):
                yield 0
        self.assertEqual(sorted(streaming_top_k(gen(), 100)), sorted([0 for _ in range(100)]))

    def test_random_large(self):
        def gen_list():
            ints = []
            for _ in range(2000):
                ints += list(range(100))
            random.shuffle(ints)
            return ints
        def gen(l):
            return iter(l)
        l = gen_list()
        sl = sorted(l)
        for i in range(100):
            print(f"Test case: {i}")
            k = random.randint(1, 1000)
            self.assertEqual(sorted(streaming_top_k(gen(l), k)), sl[-k:])

if __name__ == '__main__':
    unittest.main()