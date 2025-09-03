
from dataclasses import dataclass
from os import EX_PROTOCOL
from copy import deepcopy
from typing import Dict, Set, Tuple

@dataclass
class Cell:
    v: int|None
    op1: int|None
    op2: int|None
    impacts: Set[int]
    r: int

class CycleDetected(Exception):
    pass

class Sheet:
    def __init__(self):
        self.name_idx = {}
        self.cells = []
    
    def calc_cell(self, c: Cell):
        if c.v is not None:
            c.r = c.v
        elif c.op1 is not None:
            c.r = self.cells[c.op1].r + (self.cells[c.op2].r if c.op2 is not None else 0)
        else:
            raise RuntimeError(f"Invalid cell {c}")
    
    def _bfs(self, start: int, calculate: bool) -> Set[int]:
        reached = set([start])
        q = [start]
        while len(q) > 0:
            v = q[0]
            q = q[1:]
            for next in self.cells[v].impacts:
                if calculate:
                    self.calc_cell(self.cells[next])
                if next not in reached:
                    reached.add(next)
                    q.append(next)
        return reached

    def _get_cell(self, name: str) -> Tuple[Cell, int]:
        idx = -1
        if name in self.name_idx:
            idx = self.name_idx[name]
        else:
            idx = self.name_idx[name] = len(self.cells)
            self.cells.append(Cell(v=None, op1=None, op2=None, impacts=set(), r=0))
        return self.cells[idx], idx
    
    def get_cell(self, name) -> int|None:
        c, _ = self._get_cell(name)
        return c.r
    
    def set_cell(self, name: str, f: str):
        cell, start = self._get_cell(name)

        # Remove edges
        # print(f"Removing edges to {start}: {cell}")
        for op in (cell.op1, cell.op2 if cell.op1 != cell.op2 else None):
            if op is not None:
                self.cells[op].impacts.remove(start)

        if f[0] != "=":
            v = int(f)
            cell.v = v
            cell.op1 = cell.op2 = None
            self.calc_cell(cell)
            self._bfs(start, True)
        else:
            original_cell = deepcopy(cell)
            cell.v = None
            parts = list(f[1:].split(" + "))
            assert len(parts) <= 2, parts
            cell1, op1 = self._get_cell(parts[0])
            cell2, op2 = cell1, op1
            cell.op1, cell.op2 = op1, None
            if len(parts) == 2:
                cell2, op2 = self._get_cell(parts[1])
                cell.op2 = op2
            if start in (op1, op2):
                raise CycleDetected()
            # Add edges
            cell1.impacts.add(start)
            if cell.op2 is not None:
                cell2.impacts.add(start)
            # Detect if new edges form a cycle
            reachable = self._bfs(start, False)
            if op1 in reachable or op2 in reachable:
                self.cells[start] = original_cell
                raise CycleDetected()
            self.calc_cell(cell)
            self._bfs(start, True)

if __name__ == "__main__":
    ss = Sheet()
    ss.set_cell('A1', '1')
    ss.set_cell('A3', '10')
    assert ss.get_cell("A1") == 1, ss.get_cell("A1")
    assert ss.get_cell("A3") == 10, ss.get_cell("A3")


    ss.set_cell('A2', '=A1 + A3') # 11
    assert ss.get_cell("A2") == 11, ss.get_cell("A2")

    try:
        ss.set_cell("A3", "=A1 + A2")
    except CycleDetected:
        pass
    else:
        raise AssertionError("Expect a CycleDetected exception")
    
    assert ss.get_cell("A3") == 10, ss.get_cell("A3")

    ss.set_cell("A3", "=A1 + A1")
    assert ss.get_cell("A3") == 2, ss.get_cell("A3")
    assert ss.get_cell("A1") == 1, ss.get_cell("A1")
    assert ss.get_cell("A2") == 3, ss.get_cell("A2")

    
    ss.set_cell('B3', '2')
    ss.set_cell('B1', '=B2 + B3')
    assert ss.get_cell("B1") == 2, ss.get_cell("B1")
    ss.set_cell('B2', '=A2 + A1') # 11 + 1 = 12
    assert ss.get_cell("B2") == 4, ss.get_cell("B2")
    ss.set_cell("A3", "=A1 + B3")
    assert ss.get_cell("A1") == 1
    assert ss.get_cell("A3") == 3
    assert ss.get_cell("A2") == 4
    assert ss.get_cell("B3") == 2
    assert ss.get_cell("B2") == 5
    assert ss.get_cell("B1") == 7