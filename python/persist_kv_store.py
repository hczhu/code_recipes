from collections import defaultdict
import struct
from typing import NamedTuple, Tuple
from io import BytesIO
from bisect import bisect_right


class KvCodec:
    I_FMT = "Q"
    I_LEN = struct.calcsize(I_FMT)
    S_FMT = "utf-8"

    def __init__(self):
        # print(f"{self.I_FMT} {self.I_LEN}")
        pass

    @classmethod
    def encode_kvs(cls, kvs: list[Tuple[str, str]], out: BytesIO) -> None:
        def write_bytes(bs: bytes) -> None:
            written = out.write(bs)
            if written != len(bs):
                raise RuntimeError("Failed to write {len(bs)} bytes.")
        
        write_bytes(struct.pack(cls.I_FMT, len(kvs)))

        def write_str(s: str) -> None:
            b = s.encode(cls.S_FMT)
            write_bytes(
                struct.pack(f"{cls.I_FMT}{len(b)}s", len(b), b)
            )

        for k, v in kvs:
            write_str(k)
            write_str(v)
    
    @classmethod
    def decode_kvs(cls, input: BytesIO) -> list[Tuple[str, str]]:
        def read_next_len() -> int:
            return struct.unpack(cls.I_FMT, input.read(cls.I_LEN))[0]

        num_kvs = read_next_len()
        # print(f"Decoding {num_kvs} kvs...")
        res = []
        def read_next_str() -> str:
            l = read_next_len()
            return input.read(l).decode(cls.S_FMT)

        for i in range(num_kvs):
            res.append((read_next_str(), read_next_str()))

        return res

class TimedValue(NamedTuple):
    ts: int
    v: str

    def __lt__(self, r: "TimedValue") -> bool:
        return self. ts < r.ts

class KvStore:
    def __init__(self):
        self.cc = KvCodec()
        self.d: dict[str, list[TimedValue]] = defaultdict(list)

    def persist(self, out: BytesIO) -> None:
        kvs = []
        for k, l in self.d.items():
            kvs.append((k, l[-1].v))
        self.cc.encode_kvs(kvs, out)

    def load(self, input: BytesIO) -> None:
        kvs = self.cc.decode_kvs(input)
        self.d.clear()
        for k, v in kvs:
            self.d[k].append(TimedValue(ts=0, v=v))

    def set(self, k: str, v: str, ts: int) -> None:
        self.d[k].append(
            TimedValue(ts=ts, v=v)
        )

    def get(self, k: str, ts: int) -> str|None:
        l = self.d.get(k, None)
        if l is None:
            return None 
        # l[pos] is the first one > ts
        pos = bisect_right(l, TimedValue(ts=ts, v=""))
        return l[pos - 1].v if pos > 0 else None

if __name__ == "__main__":
    kvs = [
        ("aa", "c"),
        ("", "ddd"),
        ("xx我们", "哈哈"),
    ]
    assert bisect_right([0], 1) == 1
    cc = KvCodec()
    out = BytesIO()
    cc.encode_kvs(kvs, out)
    out.seek(0)
    decoded_kvs = cc.decode_kvs(out)
    assert kvs == decoded_kvs

    kv = KvStore()
    clock = 0
    kv.set("a", "a", clock)
    kv.set("b", "b", clock)
    assert kv.get("a", 0) == "a"
    assert kv.get("a", 1) == "a"
    assert kv.get("a", -1) is None

    assert kv.get("b", 0) == "b"
    assert kv.get("b", 1) == "b"
    
    assert kv.get("c", 1) is None
    clock += 1
    kv.set("a", "a1", clock)
    kv.set("c", "c", clock)

    assert kv.get("c", 0) is None
    assert kv.get("c", 1) == "c"

    assert kv.get("a", 0) == "a"
    assert kv.get("a", 1) == "a1"
    assert kv.get("a", 2) == "a1"

    clock += 1
    kv.set("b", "b1", clock)
    assert kv.get("b", 0) == "b"
    assert kv.get("b", 1) == "b"
    assert kv.get("b", 2) == "b1"
    assert kv.get("b", 3) == "b1"

    out = BytesIO()
    kv.persist(out)
    out.seek(0)
    kv2 = KvStore()
    kv2.load(out)
    assert kv2.get("a", 0) == "a1"
    assert kv2.get("a", 1) == "a1"
    assert kv2.get("a", 2) == "a1"
    assert kv2.get("a", 3) == "a1"
    assert kv2.get("b", 0) == "b1"
    assert kv2.get("b", 1) == "b1"
    assert kv2.get("b", 2) == "b1"
    assert kv2.get("b", 3) == "b1"
    assert kv2.get("c", 0) == "c"