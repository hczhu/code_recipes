from distutils.command.build import build
import heapq


class Solution(object):
    def getSkyline(self, buildings):
        """
        :type buildings: List[List[int]]
        :rtype: List[List[int]]
        """
        X = set()
        for x1, x2, _ in buildings:
            X.add(x1)
            X.add(x2)
        X = list(X)
        X.sort()

        p = 0
        heap = []
        res = []
        for x in X:
            while len(heap) > 0 and heap[0][1] <= x:
                heapq.heappop(heap)
            while p < len(buildings) and buildings[p][0] == x:
                x1, x2, h = buildings[p]
                p += 1
                heapq.heappush(heap, (-h, x2))
            y = -heap[0][0] if len(heap) > 0 else 0
            # print(x, ": ", y, " - ", heap)
            res.append([x, y])

        prev_h = -1
        p = 0
        for x, h in res:
            if prev_h != h:
                res[p] = [x, h]
                p += 1
                prev_h = h
        return res[:p]


s = Solution()
assert s.getSkyline(
    [[2, 9, 10], [3, 7, 15], [5, 12, 12], [15, 20, 10], [19, 24, 8]]
) == [[2, 10], [3, 15], [7, 12], [12, 0], [15, 10], [20, 8], [24, 0]]


assert s.getSkyline([[0, 2, 3], [2, 5, 3]]) == [[0, 3], [5, 0]]
