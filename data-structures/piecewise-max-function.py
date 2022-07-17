from dataclasses import dataclass

@dataclass
class Segment:
    """
    a segment of line y = a * x + b in [x1, x2]
    where a >= 0
    """
    a: int
    b: int
    x1: float
    x2: float

    def x2_below(self, a, b):
        # Note that 1e30 < 1e30 + 1e-10  is False!
        # 1e30 > 1e30 + 1e-10 is also False
        return (self.a - a) * self.x2 + (self.b - b) <= 0

    def x1_below(self, a, b):
        return (self.a - a) * self.x1 + (self.b - b) <= 0

    def intersection(self, a, b):
        return (b - self.b) / (self.a - a)

class PiecewiseMax:
    """
    Maintain a piecewise function maximizing max{
        a[i] * x + b[i]  for any i
    } where a[i] >= 0 and x >= 0
    """
    INF = 1e40

    def __init__(self):
        # self.q holds Segment instances sorted by Segment::a
        N = 1_000_000
        self.q = [None] * (2*N + 1)
        self.head = N
        self.tail = N

    def add_first(self, a, b):
        self.q[self.tail] = Segment(a=a, b=b, x1=0.0, x2=self.INF)
        self.tail += 1
    
    def empty(self):
        return self.head == self.tail
    
    def head_seg(self):
        return self.q[self.head]

    def tail_seg(self):
        return self.q[self.tail - 1]

    def max_y_for(self, x):
        if self.empty():
            return -self.INF
        h, t = self.head, self.tail
        while h < t:
            m = (h + t) // 2
            if self.q[m].x2 < x:
                h = m + 1
            else:
                t = m
        
        return self.q[h].a * x + self.q[h].b

    def prepend(self, a, b):
        """
        Pre-condition: a <= self.q[self.head].a
        """

        while not self.empty() and self.head_seg().x2_below(a, b):
            self.head += 1

        if self.empty():
            self.add_first(a, b)
            return

        if not self.head_seg().x1_below(a, b):
            return
        
        mx = self.head_seg().intersection(a, b)

        self.head_seg().x1 = mx
        self.head -= 1
        self.q[self.head] = Segment(
            a=a, b=b, x1=0, x2=mx,
        )
    
    def append(self, a, b):
        """
        Pre-condition: a >= self.q[self.tail - 1].a
        """
        tail = self.tail
        while not self.empty() and self.tail_seg().x1_below(a, b):
            self.tail -= 1

        if self.empty():
            self.add_first(a, b)
            return
          
        if self.tail_seg().a == a:
            return
        mx = self.tail_seg().intersection(a, b)
        self.tail_seg().x2 = mx
        self.tail += 1
        self.q[self.tail - 1] = Segment(
            a=a, b=b, x1=mx, x2=self.INF,
        )

    def dump(self):
        print(self.q[self.head:self.tail])

    

        
from typing import List
# Write any import statements here


def getMaxDamageDealt(N: int, H: List[int], D: List[int], B: int) -> float:
    res = 0
    dh = [
        (D[i], H[i])
        for i in range(N)
      ]

    dh.sort(
        key = lambda dh: (dh[0], -dh[1])
    )
    # print(dh)

    pw = PiecewiseMax()
    for d, h in dh:
        # print(d, h, pw.max_y_for(h))
        res = max(res, d * h + pw.max_y_for(h))
        # print(res)
        pw.append(d, d*h)
        # pw.dump()

    pw = PiecewiseMax()
    for i in range(N - 1, -1, -1):
        d, h = dh[i][0], dh[i][1]
        # print(d, d*h, pw.max_y_for(h))
        res = max(res, d * h + pw.max_y_for(h))
        # print(res)
        pw.prepend(d, d*h)
        # pw.dump()
    return res / B

print(getMaxDamageDealt(3, [2, 1, 4], [3, 1, 2], 4))



print(getMaxDamageDealt(4, [1, 1, 2, 100], [1, 2, 1, 3], 8))



print(getMaxDamageDealt(4, [1, 1, 2, 3], [1, 2, 1, 100], 8))   

print(getMaxDamageDealt(3, [4,2,3], [1,1,1], 1)) 
        
N = 500_000
H = 1
import random
for i in range(100):
    random.seed(i)
    print(getMaxDamageDealt(N, 
     [ random.randint(1, 100) for _ in range(N) ], 
     [ random.randint(1, 100) for _ in range(N) ], 
    1
    )) 