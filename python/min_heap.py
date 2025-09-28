from typing import TypeVar, Protocol, Set, Generic, List, NamedTuple, Dict, Self
from collections.abc import Hashable
from collections import defaultdict
from bisect import bisect_left, bisect_right 
import heapq

class Comparable(Protocol):
    def __lt__(self, other) -> bool: ...

class HashableComparable(Protocol):
    def __lt__(self, other) -> bool: ...
    def __eq__(self, other) -> bool: ...
    def __hash__(self) -> int: ...

HCT = TypeVar('HCT', bound=HashableComparable)
class MinHeap(Generic[HCT]):
    def __init__(self, l: list[HCT]):
        self.h = l
        heapq.heapify(self.h)
        self.removed: Set[HCT] = set()
    
    def remove(self, e: HCT):
        self.removed.add(e)
    
    def popN(self, n: int) -> list[HCT]:
        res = []
        while len(res) < n and len(self.h) > 0:
            e = heapq.heappop(self.h)
            if e in self.removed:
               self.removed.remove(e)
            else:
                res.append(e)
        return res
    
    def push(self, l: list[HCT]) -> None:
        for e in l:
            if e in self.removed:
                self.removed.remove(e)
            else:
                heapq.heappush(self.h, e)

class TopK(Generic[HCT]):
    def __init__(self, k: int, l: list[HCT]):
        self.h = MinHeap[HCT](l)
        self.k = k
        self.top_k = self.h.popN(k)
    
    def add(self, e: HCT) -> None:
        pos = bisect_right(self.top_k, e)
        while pos < len(self.top_k):
            self.top_k[pos], e = e, self.top_k[pos]
            pos += 1
        self.top_k.append(e)
        if len(self.top_k) > self.k:
            self.h.push([self.top_k.pop()])
    
    def remove(self, e: HCT) -> None:
        pos = bisect_right(self.top_k, e)
        if pos == 0 or self.top_k[pos-1] != e:
            self.h.remove(e)
            return
        while pos < len(self.top_k):
            self.top_k[pos - 1] = self.top_k[pos]
            pos += 1
        self.top_k.pop()
        top_heap = self.h.popN(1)
        if top_heap:
            self.add(top_heap[0])
    
    def get_top_k(self) -> List[HCT]:
        return self.top_k

CT = TypeVar("CT", bound=Comparable)
class HctIndex(Generic[CT]):
    Elements: list[CT] = []

    @classmethod
    def setCTs(cls, elements: list[CT]):
        cls.Elements = elements

    def __init__(self, idx: int) -> None:
        assert idx < len(self.Elements)
        self.idx = idx
    
    def __lt__(self, r: Self) -> bool:
        return self.Elements[self.idx] < self.Elements[r.idx]

    def __eq__(self, r: Self) -> bool:
        return self.idx == r.idx
    
    def __hash__(self) -> int:
        return self.idx

    def get_idx(self) -> int:
        return self.idx

class _Movie(NamedTuple):
    price: int
    shop: int
    movie: int

class Movie(_Movie, Comparable):
    def __lt__(self, r: Self)-> bool:
        return (self.price, self.shop, self.movie) < (r.price, r.shop, r.movie)


class MovieRentingSystem:

    def __init__(self, n: int, entries: List[List[int]]):
        self.movies = [
            Movie(price=price, shop=shop, movie=movie)
            for shop, movie, price in entries
        ]
        self.MoiveIndexType = HctIndex[Movie]
        self.MoiveIndexType.setCTs(self.movies)
        self.rented_movies = TopK[HctIndex[Movie]](5, [])
        self.unrented_movies: Dict[int, TopK[HctIndex[Movie]]] = {}
        self.inverted_index = defaultdict(dict)
        for idx, m in enumerate(self.movies):
            mid = m.movie
            self.inverted_index[m.shop][mid] = idx
            wrapperIdx = self.MoiveIndexType(idx)
            if mid in self.unrented_movies:
                self.unrented_movies[mid].add(wrapperIdx)
            else:
                self.unrented_movies[mid] = TopK[HctIndex[Movie]](5, [wrapperIdx])
        
        self.search_cache: Dict[int, List[int]] = {}
        self.report_cache = None

    def search(self, movie: int) -> List[int]:
        if movie in self.search_cache:
            return self.search_cache[movie]
        res = []
        if movie not in self.unrented_movies:
            return res 
        h = self.unrented_movies[movie]
        res = h.get_top_k()
        ans = [self.movies[m.get_idx()].shop for m in res]
        self.search_cache[movie] = ans
        return ans

    def rent(self, shop: int, movie: int) -> None:
        heap_ele = self.MoiveIndexType(self.inverted_index[shop][movie])
        self.unrented_movies[movie].remove(heap_ele)
        self.rented_movies.add(heap_ele)
        self.report_cache = None
        self.search_cache.pop(movie, None)

    def drop(self, shop: int, movie: int) -> None:
        heap_ele = self.MoiveIndexType(self.inverted_index[shop][movie])
        self.unrented_movies[movie].add(heap_ele)
        self.rented_movies.remove(heap_ele)
        self.report_cache = None
        self.search_cache.pop(movie, None)
        
    def report(self) -> List[List[int]]:
        if self.report_cache is not None:
            return self.report_cache
        res = self.rented_movies.get_top_k()
        self.report_cache = [
            [self.movies[wi.get_idx()].shop, self.movies[wi.get_idx()].movie]
           for wi in res 
        ]
        return self.report_cache
        
if __name__ == "__main__":
    h = MinHeap([2, 3, 1,5, 7, 8])
    h.remove(2)
    mins = h.popN(3)
    assert mins == [1, 3, 5], mins
    h.push(mins[1:])
    h.remove(1)
    h.remove(5)
    assert h.popN(2) ==[3, 7]

    class IntWrap(int):
        pass
    
    h = MinHeap([IntWrap(10)])

    mrs = MovieRentingSystem(3, [[0,1,5],[0,2,6],[0,3,7],[1,1,4],[1,2,7],[2,1,5]])
    assert mrs.search(1) == [1, 0, 2], mrs.search(1)
    mrs.rent(0, 1)
    mrs.rent(1, 2)
    assert mrs.report() == [[0, 1], [1, 2]], mrs.report()
    assert mrs.search(2) == [0], mrs.search(2)
    mrs.drop(1, 2)
    assert mrs.search(2) == [0, 1], mrs.search(2)