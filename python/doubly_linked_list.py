from typing import TypeVar, Generic, Self

T = TypeVar("T")


class DoubleLinks(Generic[T]):
    def __init__(self, t: T | None = None, prev: Self | None = None, next: Self | None = None):
        self.prev = prev
        self.next = next
        self.t = t

    def get_data(self) -> T | None:
        return self.t

    # Return the node preceding it. If it's None, return the next.
    def remove(self) -> Self | None:
        prev, next = self.prev, self.next
        if prev is not None:
            prev.next = next
            if next is not None:
                next.prev = prev
            return prev
        if next is not None:
            next.prev = None
        return next

    # Insert after self. Return the new node.
    def insert(self, dl: Self) -> Self:
        next = self.next
        self.next = dl
        dl.prev = self
        dl.next = next
        if next is not None:
            next.prev = dl
        return dl

    def __iter__(self) -> "DoubleLinksIter":
        return DoubleLinksIter(self)


class DoubleLinksIter(Generic[T]):
    def __init__(self, dl: DoubleLinks[T] | None):
        self.dl = dl

    def __next__(self) -> T | None:
        if self.dl is None:
            raise StopIteration
        res = self.dl.t
        self.dl = self.dl.next
        return res


if __name__ == "__main__":
    p0 = DoubleLinks[int](0)
    p1 = p0.insert(DoubleLinks(1))
    assert list(p0) == [0, 1], list(p0)
    p2 = p1.insert(DoubleLinks(2))
    assert list(p0) == [0, 1, 2], list(p0)
    p1.remove()
    assert list(p0) == [0, 2], list(p0)
    p3 = p0.insert(DoubleLinks(3))
    assert list(p0) == [0, 3, 2], list(p0)
    assert [t for t in p0] == [0, 3, 2], [t for t in p3]
    p0.remove()
    assert list(p3) == [3, 2], list(p3)
    p2.remove()
    assert list(p3) == [3], list(p3)
