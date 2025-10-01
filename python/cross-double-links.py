from typing import Generic, TypeVar, Self, Tuple
from doubly_linked_list import DoubleLinks

T = TypeVar("T")


class MinMaxCounters(Generic[T]):
    class Data:
        def __init__(self, t: T | None = None, head: Self | None = None, cnt: int = 0, l: Self | None = None):
            self.t = t
            self.head = head
            self.cnt = cnt
            self.l = l

    Dnode = DoubleLinks[Data]

    def __init__(self):
        self.max_head = MinMaxCounters.Dnode(t=self.Data())
        self.min_tail = MinMaxCounters.Dnode(t=self.Data())
        self.max_head.next = self.min_tail
        self.min_tail.prev = self.max_head

        self.index: dict[T, MinMaxCounters.Dnode] = {}

    def remove(self, p: Dnode) -> None:
        p.remove()
        phead = p.get_data().head
        if phead.get_data().l == p:
            phead.get_data().l = p.next
        if phead.get_data().l is None and phead != self.max_head and phead != self.min_tail:
            phead.remove()
        p.get_data().head = p.next = p.prev = None

    def add(self, p: Dnode, phead: Dnode) -> None:
        p.get_data().head = phead
        if phead.get_data().l is None:
            phead.get_data().l = p
        else:
            phead.get_data().l.insert(p)

    def inc(self, t: T) -> None:
        pt = self.index.get(t, None)
        if pt is None:
            pt = MinMaxCounters.Dnode(
                MinMaxCounters.Data(t=t, head=self.min_tail)
            )
            self.index[t] = pt

        phead = pt.get_data().head
        phead_prev = phead.prev
        new_cnt = phead.get_data().cnt + 1
        # NB: phead may have been deleted. Don't use.
        self.remove(pt)
        phead = None
        if phead_prev.get_data().cnt != new_cnt:
            phead_prev = phead_prev.insert(MinMaxCounters.Dnode(
                self.Data(cnt=new_cnt)
            ))
        self.add(pt, phead_prev)

    def dec(self, t: T) -> None:
        pt = self.index.get(t, None)
        if pt is None:
            return
        phead = pt.get_data().head
        phead_next = phead.next
        new_cnt = phead.get_data().cnt - 1
        # print(f"Dec {t} from {phead.get_data().cnt} to {new_cnt}")
        # NB: phead may have been deleted. Don't use.
        self.remove(pt)
        phead = None
        if new_cnt == 0:
            del self.index[t]
            return
        assert phead_next is not None and phead_next is not self.min_tail and phead_next is not self.max_head
        if phead_next.get_data().cnt != new_cnt:
            phead_next = phead_next.prev.insert(MinMaxCounters.Dnode(
                self.Data(cnt=new_cnt)
            ))
        self.add(pt, phead_next)

    def get_max(self) -> Tuple[T | None, int]:
        dnode = self.max_head.next
        if dnode.get_data().l is None:
            return None, 0
        return dnode.get_data().l.get_data().t, dnode.get_data().cnt

    def get_min(self) -> Tuple[T | None, int]:
        dnode = self.min_tail.prev
        if dnode.get_data().l is None:
            return None, 0
        return dnode.get_data().l.get_data().t, dnode.get_data().cnt


class AllOne:

    def __init__(self):
        self.mmc = MinMaxCounters[str]()

    def inc(self, key: str) -> None:
        self.mmc.inc(key)

    def dec(self, key: str) -> None:
        self.mmc.dec(key)

    def getMaxKey(self) -> str:
        return self.mmc.get_max()[0]

    def getMinKey(self) -> str:
        return self.mmc.get_min()[0]

        # Your AllOne object will be instantiated and called as such:
        # obj = AllOne()
        # obj.inc(key)
        # obj.dec(key)
        # param_3 = obj.getMaxKey()
        # param_4 = obj.getMinKey()

if __name__ == "__main__":
    mmc = MinMaxCounters[str]()
    mmc.inc("a")
    assert mmc.get_max() == ("a", 1)
    assert mmc.get_min() == ("a", 1), mmc.get_min()
    mmc.inc("b")
    assert mmc.get_max() == ("a", 1), mmc.get_max()
    assert mmc.get_min() == ("a", 1)
    mmc.inc("b")
    assert mmc.get_max() == ("b", 2), mmc.get_max()
    assert mmc.get_min() == ("a", 1)
    mmc.inc("b")
    assert mmc.get_max() == ("b", 3), mmc.get_max()
    assert mmc.get_min() == ("a", 1)
    mmc.inc("b")
    assert mmc.get_max() == ("b", 4), mmc.get_max()
    assert mmc.get_min() == ("a", 1)
    mmc.dec("b")
    assert mmc.get_max() == ("b", 3), mmc.get_max()
    assert mmc.get_min() == ("a", 1), mmc.get_min()
    mmc.dec("b")
    assert mmc.get_max() == ("b", 2), mmc.get_max()
    assert mmc.get_min() == ("a", 1), mmc.get_min()
    mmc.dec("b")
    assert mmc.get_max() == ("a", 1), mmc.get_max()
    assert mmc.get_min() == ("a", 1), mmc.get_min()
    mmc.dec("a")
    assert mmc.get_max() == ("b", 1), mmc.get_max()
    assert mmc.get_min() == ("b", 1), mmc.get_min()
    mmc.dec("b")
    assert mmc.get_max() == (None, 0), mmc.get_max()
    assert mmc.get_min() == (None, 0), mmc.get_min()

    mmc = MinMaxCounters[str]()
    mmc.inc("hello")
    mmc.inc("goodbye")
    mmc.inc("hello")
    mmc.inc("hello")
    assert mmc.get_max() == ("hello", 3)
    mmc.dec("hello")
    assert mmc.get_max() == ("hello", 2), mmc.get_max()

    mmc.inc("leet")
    mmc.inc("code")
    mmc.inc("leet")
    mmc.dec("hello")
    mmc.inc("leet")
    mmc.inc("code")
    mmc.inc("code")
    assert mmc.get_max() == ("leet", 3), mmc.get_max()
