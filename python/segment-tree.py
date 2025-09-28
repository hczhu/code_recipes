from dataclasses import dataclass
from turtle import update
from xmlrpc.client import boolean


@dataclass
class Segment(object):
    l: int
    # Exclusive r > l
    r: int
    max_row_seats: int = 0
    sum_row_seats: int = 0
    left: object = None
    right: object = None


def printTree(seg, indents="  "):
    print(
        f"{indents}[{seg.l}, {seg.r}) = max({seg.max_row_seats}) sum({seg.sum_row_seats})"
    )
    if seg.left is not None:
        printTree(seg.left, indents + "  ")
    if seg.right is not None:
        printTree(seg.right, indents + "  ")


def constructSegmentTree(seg, m):
    mid = (seg.l + seg.r) // 2
    if seg.l == mid:
        return

    assert mid < seg.r and seg.l < mid
    l_seg = Segment(
        l=seg.l, r=mid, max_row_seats=m, sum_row_seats=m * (mid - seg.l)
    )
    r_seg = Segment(
        l=mid, r=seg.r, max_row_seats=m, sum_row_seats=m * (seg.r - mid)
    )

    constructSegmentTree(l_seg, m)
    constructSegmentTree(r_seg, m)
    seg.left = l_seg
    seg.right = r_seg


def updateSeg(seg):
    seg.max_row_seats = max(seg.left.max_row_seats, seg.right.max_row_seats)
    seg.sum_row_seats = seg.left.sum_row_seats + seg.right.sum_row_seats


def gather(seg, k, maxRow, m):
    assert maxRow >= seg.l
    if seg.max_row_seats < k:  # or seg.fully_occupied:
        return None
    if seg.left is None:
        seg.max_row_seats -= k
        seg.sum_row_seats -= k
        return [seg.l, m - seg.sum_row_seats - k]

    l_res = gather(seg.left, k, maxRow, m)
    if l_res is not None:
        updateSeg(seg)
        return l_res

    if maxRow >= seg.right.l:
        r_res = gather(seg.right, k, maxRow, m)
        if r_res is not None:
            updateSeg(seg)
            return r_res
    return None


def can_scatter(seg, k, maxRow):
    if seg.sum_row_seats < k:
        return False
    if seg.left is None:
        assert seg.sum_row_seats >= k
        return True

    if maxRow < seg.left.r:
        return can_scatter(seg.left, k, maxRow)
    elif k <= seg.left.sum_row_seats:
        return True
    else:
        return can_scatter(seg.right, k - seg.left.sum_row_seats, maxRow)


def scatter(seg, k, maxRow):
    if seg.left is None:
        assert k <= seg.sum_row_seats, (seg, k, maxRow)
        assert seg.l <= maxRow
        seg.sum_row_seats -= k
        seg.max_row_seats -= k
        return

    if k == seg.sum_row_seats:
        assert seg.r - 1 <= maxRow
        seg.max_row_seats = seg.sum_row_seats = 0
        return

    if maxRow < seg.left.r or k <= seg.left.sum_row_seats:
        scatter(seg.left, k, maxRow)
    else:
        l_seats = seg.left.sum_row_seats
        scatter(seg.left, l_seats, maxRow)
        scatter(seg.right, k - l_seats, maxRow)

    updateSeg(seg)


class BookMyShow(object):
    def __init__(self, n, m):
        """
        :type n: int
        :type m: int
        """
        self.root = Segment(
            l=0,
            r=n,
            max_row_seats=m,
            sum_row_seats=m * n,
        )
        self.m = m
        constructSegmentTree(self.root, m)

    def gather(self, k, maxRow):
        """
        :type k: int
        :type maxRow: int
        :rtype: List[int]
        """
        res = gather(self.root, k, maxRow, self.m)
        return res if res is not None else []

    def scatter(self, k, maxRow):
        """
        :type k: int
        :type maxRow: int
        :rtype: bool
        """
        if can_scatter(self.root, k, maxRow):
            scatter(self.root, k, maxRow)
            return True
        return False


bms = BookMyShow(2, 5)
print(bms.gather(4, 0))
print(bms.gather(2, 0))

print(bms.scatter(5, 1))
print(bms.scatter(5, 1))


bms = BookMyShow(4, 5)
print(bms.scatter(6, 2))

print(bms.gather(6, 3))

print(bms.scatter(9, 1))

print("----------")

bms = BookMyShow(5, 9)

print(bms.gather(10, 1))
printTree(bms.root)
print("=========")

print(bms.scatter(3, 3))
printTree(bms.root)
print("=========")

print(bms.gather(9, 1))
printTree(bms.root)
print("=========")

print(bms.gather(10, 2))
print(bms.gather(2, 0))


# Your BookMyShow object will be instantiated and called as such:
# obj = BookMyShow(n, m)
# param_1 = obj.gather(k,maxRow)
# param_2 = obj.scatter(k,maxRow)
