from audioop import mul
from dataclasses import dataclass

@dataclass
class Segment(object):
    # For segment [l, r)
    l: int
    r: int
    # Represent a * x + b
    a: int = 1
    b: int = 0
    # The value of x
    c: int = 0

    lc: object = None
    rc: object = None

M = 1000000000 + 7

def printTree(seg, indents="  "):
    print(f"{indents}[{seg.l}, {seg.r}) = {seg.a} * {seg.c} + {seg.b}")
    if seg.lc is not None:
        printTree(seg.lc, indents + "  ")
    if seg.rc is not None:
        printTree(seg.rc, indents + "  ")


def constructSegmentTree(l, r):
    seg = Segment(l=l, r=r)
    mid = (l + r) // 2
    if seg.l == mid:
        return seg
    
    assert mid < seg.r and seg.l < mid
    seg.lc = constructSegmentTree(l, mid)
    seg.rc = constructSegmentTree(mid, r)
    return seg

def append(seg, pos, value):
    while seg.l + 1 < seg.r:
        seg = seg.lc  if pos < seg.lc.r else seg.rc
    seg.c = value
    seg.a = 1

def opForAll(seg, r, value, full_coverage_callback):
    while seg.r > r:
        assert r > seg.l, (r, seg)
        if r > seg.lc.r:
            # Cover all left child segment
            full_coverage_callback(seg.lc, value)
            seg = seg.rc
        else:
            seg = seg.lc
    full_coverage_callback(seg, value)

def query(seg, pos):
    stack = []
    while seg.l + 1 < seg.r:
        stack.append(seg)
        seg = seg.lc  if pos < seg.lc.r else seg.rc
    
    res = (seg.a * seg.c + seg.b) % M

    while len(stack) > 0:
        res = ((res * stack[-1].a) + stack[-1].b) % M
        stack.pop()
    
    return res


def growTree(root):
    assert root.l == 0, root
    new_root = Segment(l=root.l, r=root.r * 2, lc=root)
    new_root.rc = constructSegmentTree(root.r, root.r * 2)
    return new_root

class Fancy:

    def __init__(self):
        self.root = constructSegmentTree(0, 10)
        self.n = 0

    def append(self, val: int) -> None:
        self.n += 1
        if self.n > self.root.r:
            self.root = growTree(self.root)
        assert self.n <= self.root.r, self.root

        append(self.root, self.n - 1, val)

        

    def addAll(self, inc: int) -> None:
        if self.n == 0: return
        def add(seg, inc):
            seg.b = (seg.b + inc) % M
        
        opForAll(self.root, self.n, inc, add)

    def multAll(self, m: int) -> None:
        if self.n == 0: return
        def mul(seg, m):
            seg.a = (seg.a * m) % M
            seg.b = (seg.b * m) % M
        
        opForAll(self.root, self.n, m, mul)
       

    def getIndex(self, idx: int) -> int:
        if idx >= self.n:
            return -1
        return query(self.root, idx)
        

fancy = Fancy()
fancy.append(2)
printTree(fancy.root)
print("========")

fancy.addAll(3)    
fancy.append(7)    
fancy.multAll(2)  

printTree(fancy.root)
print("========")

assert fancy.getIndex(0) == 10  

fancy.addAll(3)    
fancy.append(10)   
fancy.multAll(2)   

printTree(fancy.root)
print("========")

assert fancy.getIndex(0) == 26

assert fancy.getIndex(1)  == 34
assert fancy.getIndex(2)  == 20