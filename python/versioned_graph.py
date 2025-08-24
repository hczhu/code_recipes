from bisect import bisect_right
from collections import defaultdict
from typing import DefaultDict, Iterable, NamedTuple

class Version(NamedTuple):
    v: int
    dm: bool  # deletion marker

class EdgeIterator:
    def __init__(self, edge_dict: DefaultDict[int, list[Version]], target_version: int):
        self.ends = sorted(edge_dict)  # no need for list(keys())
        self.edge_dict = edge_dict
        self.ptr = 0
        self.target_version = target_version

    def __iter__(self) -> "EdgeIterator":
        return self

    def __next__(self) -> int:
        while self.ptr < len(self.ends):
            end = self.ends[self.ptr]
            self.ptr += 1
            versions = self.edge_dict[end]
            idx = VersionedDirectedGraph.search_for_version(self.target_version, versions)
            if idx is not None and not versions[idx].dm:
                return end
        raise StopIteration

class VersionedDirectedGraph:
    def __init__(self, n: int):
        # graph[start][end] -> list of Version
        self.graph: list[DefaultDict[int, list[Version]]] = [defaultdict(list) for _ in range(n)]
        self.version = 0

    def _append_version(self, start: int, end: int, dm: bool) -> None:
        versions = self.graph[start][end]
        if not (versions and versions[-1].dm == dm):
            versions.append(Version(v=self.version, dm=dm))

    def add_edge(self, start: int, end: int) -> int:
        self.version += 1
        self._append_version(start, end, dm=False)
        return self.version

    def delete_edge(self, start: int, end: int) -> int:
        self.version += 1
        self._append_version(start, end, dm=True)
        return self.version

    def snapshot(self) -> int:
        return self.version

    @staticmethod
    def search_for_version_easy(target_version: int, versions: list[Version]) -> int | None:
        if not versions or versions[0].v > target_version:
            return None
        # use bisect_right over the list of version numbers
        idx = bisect_right([x.v for x in versions], target_version) - 1
        return idx if idx >= 0 else None

    @staticmethod
    def search_for_version(target_version: int, versions: list[Version]) -> int | None:
        low, high = 0, len(versions) - 1
        if versions[0].v > target_version:
            return None
        # Invariant: versions[low].v <= target_version
        while low < high:
            # Invariant: low < mid <= high
            mid = (low + high) // 2 + 1
            if versions[mid].v <= target_version:
                low = mid
            else:
                high = mid - 1
        return low

    def has_edge(self, start: int, end: int, v: int) -> bool:
        if start < 0 or start >= len(self.graph):
            return False
        if end not in self.graph[start]:
            return False
        versions = self.graph[start][end]
        idx = self.search_for_version(v, versions)
        return idx is not None and not versions[idx].dm

    def get_edge_iterator(self, start: int, v: int) -> Iterable[int]:
        if start < 0 or start >= len(self.graph):
            return iter(())
        return EdgeIterator(self.graph[start], v)

def bfs(g: VersionedDirectedGraph, start: int, snapshot: int, max_dist: int) -> list[set[int]]:
    res = [{start}]
    visited = {start}
    for d in range(max_dist):
        frontier: set[int] = set()
        for node in res[d]:
            for u in g.get_edge_iterator(node, snapshot):
                if u not in visited:
                    visited.add(u)
                    frontier.add(u)
        res.append(frontier)
    return res

def common_connections(g: VersionedDirectedGraph, a: int, b: int, snapshot: int) -> list[int]:
    """Find common outgoing connections between nodes a and b at given snapshot"""
    res = []
    b_edges = list(g.get_edge_iterator(b, snapshot))
    ptr_b = 0
    for a_edge in g.get_edge_iterator(a, snapshot):
        while ptr_b < len(b_edges) and b_edges[ptr_b] < a_edge:
            ptr_b += 1
        if ptr_b == len(b_edges):
            break
        if b_edges[ptr_b] == a_edge:
            res.append(a_edge)
    return res

if __name__ == "__main__":
    g = VersionedDirectedGraph(4)
    g.add_edge(0, 2)
    g.add_edge(0, 3)
    s1 = g.snapshot()
    g.add_edge(3, 0)
    s2 = g.snapshot()
    g.add_edge(3, 0)

    assert g.has_edge(0, 2, s1)
    assert not g.has_edge(2, 0, s2)
    assert g.has_edge(0, 3, s1)
    assert not g.has_edge(3, 0, s1)
    assert g.has_edge(3, 0, s2)
    assert g.has_edge(0, 3, s2)

    s3 = g.delete_edge(0, 3)
    assert g.has_edge(0, 3, s2)
    assert not g.has_edge(0, 3, s3)

    g.delete_edge(3, 0)
    s4 = g.snapshot()
    assert not g.has_edge(3, 0, s4)
    assert g.has_edge(3, 0, s3)

    s5 = g.add_edge(3, 0)
    assert not g.has_edge(3, 0, s4)
    assert g.has_edge(3, 0, s3)
    assert g.has_edge(3, 0, s5)
    assert g.has_edge(3, 0, s2)
    assert not g.has_edge(3, 0, s1)

    et1 = g.get_edge_iterator(0, s1)
    assert list(et1) == [2, 3]
    et3 = g.get_edge_iterator(0, s3)
    assert list(et3) == [2]

    g.add_edge(3, 1)
    s6 = g.add_edge(0, 3)

    assert bfs(g, 0, s6, 0) == [set({0})]

    assert g.has_edge(0, 3, s6)
    assert bfs(g, 0, s6, 1) == [set({0}), set({2, 3})], bfs(g, 0, s6, 1)

    assert bfs(g, 0, s6, 2) == [set({0}), set({2, 3}), set({1})], bfs(g, 0, s6, 2)

    s7 = g.add_edge(1, 2)
    assert common_connections(g, 0, 1, s7) == [2]
    assert common_connections(g, 1, 2, s7) == []
    assert common_connections(g, 0, 3, s7) == []
    g.add_edge(3, 2)
    s8 = g.add_edge(0, 1)
    assert common_connections(g, 0, 3, s8) == [1, 2], common_connections(g, 0, 3, s8)