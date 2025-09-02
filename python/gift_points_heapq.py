import heapq
from typing import NamedTuple, Tuple
from dataclasses import dataclass
from copy import deepcopy
import bisect

_debug = False
def dprint(*args):
    if _debug:
        print(*args)

@dataclass
class Points():
    # Inclusive
    start: int
    # Inclusive
    end: int
    points: int

    def __lt__(self, other: "Points") -> bool:
        return self.end < other.end

@dataclass
class Use():
    ts: int
    points: int
    def __lt__(self, other: "Use") -> bool:
        return self.ts < other.ts

class CantMeetUses(Exception):
    pass

class GiftPoints:
    def __init__(self):
        self.points: list[Points] = []
        self.uses: list[Use] = []
    
    def add(self, points: int, start: int, end: int) -> "GiftPoints":
        self.points.append(Points(start=start, end=end, points=points))
        return self
    
    def use(self, ts: int, points: int) -> "GiftPoints":
        self.uses.append(Use(
            ts = ts,
            points = points,
        ))
        return self

    @staticmethod
    def _scan(p: list[Points], u: list[Use]) -> list[Points]:
        p = sorted(p, key=lambda x: x.start)
        u = sorted(u, key=lambda x: x.ts)
        pp = 0
        heap: list[Points]= []
        for us in u:
            while pp < len(p) and p[pp].start <= us.ts:
                heapq.heappush(heap, p[pp])
                pp += 1
            needs = us.points
            while needs > 0 and len(heap) > 0:
                quota = heapq.heappop(heap)
                dprint(f"Poped {quota} for {us}")
                if quota.end >= us.ts:
                    s = min(quota.points, needs)
                    needs -= s
                    quota.points -= s
                    if quota.points > 0:
                        heapq.heappush(heap, quota)
                dprint(f"Used {quota} for {us}")
            if needs > 0:
                raise CantMeetUses()
        return p

    def query(self, ts: int) -> int:
        points = deepcopy(self.points)
        uses = deepcopy(self.uses)
        uses = sorted(uses, key=lambda x: x.ts)
        cutoff_idx = bisect.bisect_right(uses, Use(ts=ts, points=0))
        dprint(f"==== scanning left for {ts}: {uses}=====")
        points = self._scan(points, uses[0:cutoff_idx])

        for p in points:
            p.start, p.end = -p.end, -p.start
        
        uses = uses[cutoff_idx:]
        for u in uses:
            u.ts = -u.ts
        dprint(f"==== scanning right for {ts}: {uses}=====")
        points = self._scan(points, uses)

        ans = 0
        for p in points:
            p.start, p.end = -p.end, -p.start
            if p.start <= ts and ts <= p.end:
                ans += p.points
        return ans

if __name__ == "__main__":
    heap = []
    heapq.heappush(heap, (3, -3))
    heapq.heappush(heap, (1, -1))
    heapq.heappush(heap, (2, -2))
    x, y = heapq.heappop(heap)
    assert x == 1 and y == -1

    assert heapq.heappop(heap) == (2, -2)
    assert heapq.heappop(heap) == (3, -3)

    gp = GiftPoints()
    gp.add(5, 0, 4)
    assert gp.query(0) == 5
    gp.use(0, 1)
    assert gp.query(0) == 4
    gp.use(1, 1)
    assert gp.query(3) == 3
    gp.use(2, 1)
    assert gp.query(3) == 2
    gp.use(3, 1)
    assert gp.query(4) == 1
    gp.use(4, 1)
    assert gp.query(4) == 0
    assert gp.query(2) == 0
    assert gp.query(0) == 0

    gp = GiftPoints()
    gp.add(5, 0, 3).add(4, 1, 4).add(3, 3, 6).use(3, 7)
    assert gp.query(0) == 5
    assert gp.query(1) == 5
    assert gp.query(2) == 5
    assert gp.query(3) == 5

    assert gp.query(4) == 5
    assert gp.query(5) == 3
    assert gp.query(6) == 3

    gp.use(4, 2)
    assert gp.query(0) == 3
    assert gp.query(1) == 3
    assert gp.query(2) == 3
    assert gp.query(3) == 3

    assert gp.query(4) == 3
    assert gp.query(5) == 3
    assert gp.query(6) == 3

    gp.use(1, 1)
    assert gp.query(0) == 2
    assert gp.query(1) == 2
    assert gp.query(2) == 2
    assert gp.query(3) == 2

    assert gp.query(4) == 2
    assert gp.query(5) == 2
    assert gp.query(6) == 2

    gp.add(1, -1, 1)
    assert gp.query(0) == 3
    assert gp.query(1) == 3
    assert gp.query(2) == 3
    assert gp.query(3) == 3

    assert gp.query(4) == 3
    assert gp.query(5) == 3
    assert gp.query(6) == 3