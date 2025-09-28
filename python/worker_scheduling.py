import unittest
import heapq
"""
A worker can work on days in [a, b] and can produce c units of work. The worker can be assigned 
any unit of work on any day in [a, b] as long as the total amount of work assigned to the worker cannot exceed c.

A project (x, y) requires y units of work on day x. The project can be assigned to any workers.

The scheduler tries to assign a series of projects to workers and meet the requirement of each project.

"""
class Scheduler:
    def __init__(self) -> None:
        # A worker is represented by a tuple (a, b, c)
        self.workers = []
        # A project is represented by a tuple (x, y)
        self.projects = []

    def addWorker(self, a: int, b: int, c: int) -> None:
        self.workers.append((a, b, c))

    def addProject(self, x, y) -> None:
        self.projects.append((x, y))

    # How many units of work at most can be assigned to a new project on day d with the current workers and projects?
    def maxUnitsOnDay(self, x) -> int:
        maxSum = sum([
            c for a, b, c in self.workers if a <= x and x <= b
        ])
        low, high = 0, maxSum
        while low < high:
            mid = (low + high + 1) // 2
            if self.feasible(x, mid):
                low = mid
            else:
                high = mid - 1
        return low
    # Is it possible to assign a project (x, y) to the workers with current projects?
    # A greedy algorithm is used to check if the workers can meet the requirements of all the current projects plus the new project (x, y).
    # 1. Sort all the workers by their starting day.
    # 2. Sort all the projects by their day.
    # 3. For each project (x, y)
    #    3.A Find all existing workers that can work on day x.
    #    3.B Sort the workers by their ending day. It's always optimal to assign a project to the worker with the earliest ending day.
    #    3.C Assign the project's units to the workers in the list in the order of their ending day. If not enough units can be assigned, return False.
    def feasible(self, x, y) -> bool:
        workers = sorted(self.workers)
        projects = sorted([(a, c) for a, c in self.projects] + [(x, y)])
        # The smallest one is at the top
        heap = []
        # pw points to workers[pw]
        pw = 0
        def move(target_day: int, required_units: int) -> int:
            units = 0
            nonlocal pw, heap, workers
            while pw < len(workers) and workers[pw][0] <= target_day:
                if workers[pw][1] >= target_day:
                    heapq.heappush(heap, [workers[pw][1], workers[pw][2]])
                pw += 1
            while len(heap) > 0 and units < required_units:
                if heap[0][0] < target_day:
                    heapq.heappop(heap)
                else:
                    delta = min(required_units - units, heap[0][1])
                    units += delta
                    heap[0][1] -= delta
                    if heap[0][1] == 0:
                        heapq.heappop(heap)
            return units

        for a, c in projects:
            units = move(a, c)
            if units < c:
                return False
        return True
        

class TestScheduler(unittest.TestCase):
    def testHeap(self):
        heap = []
        heapq.heappush(heap, 3)
        heapq.heappush(heap, 0)
        self.assertEqual(heap[0], 0)

    def testOverlapping(self):
        q = Scheduler()
        q.addWorker(1, 3, 2)
        q.addWorker(2, 4, 2)
        self.assertEqual(q.maxUnitsOnDay(2), 4)
        q.addProject(2, 3)
        self.assertEqual(q.maxUnitsOnDay(2), 1)
        self.assertEqual(q.maxUnitsOnDay(1), 1)
        self.assertEqual(q.maxUnitsOnDay(4), 1)
        self.assertEqual(q.maxUnitsOnDay(3), 1)
        self.assertEqual(q.maxUnitsOnDay(5), 0)
        self.assertEqual(q.maxUnitsOnDay(0), 0)

    def testOverlapping3(self):
        q = Scheduler()
        q.addWorker(1, 3, 2)
        q.addWorker(2, 4, 1)
        q.addWorker(3, 5, 3)
        self.assertEqual(q.maxUnitsOnDay(3), 6)
        q.addProject(3, 2)
        self.assertEqual(q.maxUnitsOnDay(1), 2)
        self.assertEqual(q.maxUnitsOnDay(2), 3)
        self.assertEqual(q.maxUnitsOnDay(3), 4)
        self.assertEqual(q.maxUnitsOnDay(4), 4)
        self.assertEqual(q.maxUnitsOnDay(5), 3)
        self.assertEqual(q.maxUnitsOnDay(6), 0)
        q.addProject(4, 3)
        self.assertEqual(q.maxUnitsOnDay(1), 1)
        self.assertEqual(q.maxUnitsOnDay(2), 1)
        self.assertEqual(q.maxUnitsOnDay(3), 1)
        self.assertEqual(q.maxUnitsOnDay(4), 1)
        self.assertEqual(q.maxUnitsOnDay(5), 1)
    
    def testSimple(self):
        q = Scheduler()
        q.addWorker(1, 4, 3)
        q.addWorker(6, 10, 4)
        q.addWorker(3, 8, 2)
        self.assertEqual(q.maxUnitsOnDay(2), 3)
        self.assertEqual(q.maxUnitsOnDay(3), 5)
        self.assertEqual(q.maxUnitsOnDay(4), 5)
        self.assertEqual(q.maxUnitsOnDay(5), 2)
        self.assertEqual(q.maxUnitsOnDay(6), 6)
        self.assertEqual(q.maxUnitsOnDay(7), 6)
        self.assertEqual(q.maxUnitsOnDay(8), 6)
        self.assertEqual(q.maxUnitsOnDay(9), 4)
        self.assertEqual(q.maxUnitsOnDay(10), 4)

        q.addProject(5, 1)
        self.assertEqual(q.maxUnitsOnDay(2), 3)
        self.assertEqual(q.maxUnitsOnDay(3), 4)
        self.assertEqual(q.maxUnitsOnDay(4), 4)
        self.assertEqual(q.maxUnitsOnDay(5), 1)
        self.assertEqual(q.maxUnitsOnDay(6), 5)
        self.assertEqual(q.maxUnitsOnDay(7), 5)
        self.assertEqual(q.maxUnitsOnDay(8), 5)
        self.assertEqual(q.maxUnitsOnDay(9), 4)
        self.assertEqual(q.maxUnitsOnDay(10), 4)   

        q.addProject(3, 2)
        self.assertEqual(q.maxUnitsOnDay(1), 2)
        self.assertEqual(q.maxUnitsOnDay(2), 2)
        self.assertEqual(q.maxUnitsOnDay(3), 2)
        self.assertEqual(q.maxUnitsOnDay(4), 2)
        self.assertEqual(q.maxUnitsOnDay(5), 1)
        self.assertEqual(q.maxUnitsOnDay(6), 5)
        self.assertEqual(q.maxUnitsOnDay(7), 5)
        self.assertEqual(q.maxUnitsOnDay(8), 5)
        self.assertEqual(q.maxUnitsOnDay(9), 4)
        self.assertEqual(q.maxUnitsOnDay(10), 4)  

if __name__ == "__main__":
    unittest.main()
