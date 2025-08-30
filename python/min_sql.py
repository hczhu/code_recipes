from types import NoneType
from typing import Tuple
from enum import Enum
from functools import total_ordering


class TableAlreadyExists(Exception):
    pass
class ColumnAlreadyExists(Exception):
    pass
class NoSuchColumn(Exception):
    pass
class NoSuchCmp(Exception):
    pass

@total_ordering
class Value:
    def __init__(self, v: int|str|None = None):
        self.v = v
    def __str__(self):
        return str(self.v)
    def __repr__(self) -> str:
        return repr(self.v)
    def __lt__(self, other: "Value") -> bool:
        if self.v is None:
            return other.v is not None
        if other.v is None:
            return False
        return self.v < other.v
    
    def __eq__(self, other: "Value") -> bool:
        return self.v == other.v
    
class Table:
    def __init__(self, table_name: str, columns: list[str]):
        self.name = table_name
        self.rows = []
        self.n = len(columns)
        self.col_index = {}
        for i, col in enumerate(columns):
            if col in self.col_index:
                raise ColumnAlreadyExists()
            self.col_index[col] = i
 
    def insert(self, cols: list[str], vals: list[str|int]):
        self.rows.append([Value()] * self.n)
        for c, v in zip(cols, vals):
            if c not in self.col_index:
                raise NoSuchColumn()
            self.rows[-1][self.col_index[c]] = Value(v)
    
    def select(self, cols: list[str]) -> "Query":
        q = Query(self).select(cols)
        return q

class LogicOp(Enum):
    AND = 1
    OR = 2

class Query:
    def __init__(self, t: Table):
        self.t = t
        self.selects = []
        self.wheres = []
        self.op = LogicOp.AND
        self.orders = []
        self.order_desc = False

    def select(self, cols: list[str]) -> "Query":
        for c in cols:
            if c not in self.t.col_index:
                raise NoSuchColumn()
        self.selects = [
            self.t.col_index[c] for c in cols
        ]
        return self
    
    def where(self, conds: list[Tuple[str, str, str|int]], op: LogicOp = LogicOp.AND) -> "Query":
        self.wheres = [
            (
                self.t.col_index[cond[0]],
                cond[1],
                cond[2]
            ) for cond in conds
        ]
        self.op = op
        return self

    def match_row(self, row: list[str|int]) -> bool:
        for c_idx, op, rv in self.wheres:
            # lv can be None
            lv = row[c_idx]
            rv = Value(rv)
            if not eval(f"lv {op} rv"):
                return False
        return True
    
    def order_by(self, cols: list[str], desc: bool =False) -> "Query":
        self.orders = [
            self.t.col_index[c] for c in cols
        ]
        self.order_desc = desc
        return self
    
    def commit(self) -> list[list[str|int]]:
        res = []
        for row in self.t.rows:
            sort_key = [
                row[i] for i in self.orders
            ]
            if self.match_row(row):
                res.append((
                    sort_key,
                    [ row[i].v for i in self.selects ]
                ))

        # print([ keys for keys, _ in res])
        res = sorted(res, key=lambda t: t[0], reverse=self.order_desc)
        # print([ keys for keys, _ in res])
        return [ r for _, r in res ]

if __name__ == "__main__":
    a, b = None, None
    assert a is b
    t = Table("t", ["name", "value", "size", "cat"])
    assert t.col_index["name"] == 0
    t.insert(["name", "size", "cat"], ["xx", 100, "b"])
    t.insert(["name", "size", "cat"], ["yy", 300, "ab"])
    t.insert(["name", "value", "cat"], ["xx", 200, "a"])
    t.insert(["name", "value", "cat"], ["yy", 100, "ba"])
    assert len(t.rows) == 4
    assert t.select(["name", "size", "cat", "value"]).commit() == [
        ['xx', 100, 'b', None],
        ['yy', 300, 'ab', None],
        ['xx', None, 'a', 200],
        ['yy', None, 'ba', 100],
    ], t.select(["name", "size", "cat", "value"]).commit()
    
    assert t.select(["name", "size"]).commit() == [
        ["xx", 100],
        ["yy", 300],
        ["xx", None],
        ["yy", None],
    ]
    assert t.select(["name", "size"]).order_by(["size"]).commit() == [
        ["xx", None],
        ["yy", None],
        ["xx", 100],
        ["yy", 300],
    ], t.select(["name", "size"]).order_by(["size"]).commit()

    assert t.select(["name", "size"]).order_by(["value"]).commit() == [
        ["xx", 100],
        ["yy", 300],
        ["yy", None],
        ["xx", None],
    ], t.select(["name", "size"]).order_by(["value"]).commit()

    assert t.select(["name", "size"]).order_by(["name", "size"]).commit() == [
        ["xx", None],
        ["xx", 100],
        ["yy", None],
        ["yy", 300],
    ], t.select(["name", "size"]).order_by(["name", "size"]).commit()

    assert t.select(["name", "size"]).where([("size", "<", 200)]).order_by(["name", "size"]).commit() == [
        ["xx", None],
        ["xx", 100],
        ["yy", None],
    ], t.select(["name", "size"]).where([("size", "<", 200)]).order_by(["name", "size"]).commit()
    
    assert t.select(["name", "size", "cat"]).where([("size", "<", 400)]).order_by(["name", "cat"], desc=True).commit() == [
        ["yy", None, "ba"],
        ["yy", 300, "ab"],
        ["xx", 100, "b"],
        ["xx", None, "a"],
    ], t.select(["name", "size", "cat"]).where([("size", "<", 400)]).order_by(["name", "cat"], desc=True).commit()
    
    assert t.select(["name", "size", "cat", "value"]).where([("size", " >= ", 100), ("cat", ">=", "b"), ("value", " != ", None)]).order_by(["name", "cat"], desc=True).commit() == [
    ], t.select(["name", "size", "cat", "value"]).where([("size", " >= ", 100), ("cat", ">=", "b"), ("value", " != ", None)]).order_by(["name", "cat"], desc=True).commit()
    
    assert t.select(["name", "size", "cat", "value"]).where([ ("cat", ">=", "b"), ("value", " != ", None)]).order_by(["name", "cat"], desc=True).commit() == [
        ['yy', None, 'ba', 100]
    ], t.select(["name", "size", "cat", "value"]).where([("cat", ">=", "b"), ("value", " != ", None)]).order_by(["name", "cat"], desc=True).commit()