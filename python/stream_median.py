import heapq

class MedianFinder:

    def __init__(self):
        # max_heap holds the smaller half
        # The numbers are negnated
        self.max_heap = []
        # min_heap holds the larger half
        self.min_heap = [10**6]

    def addNum(self, num: int) -> None:
        if num < self.min_heap[0]:
            heapq.heappush(self.max_heap, -num)
        else:
            heapq.heappush(self.min_heap, num)
        
        while len(self.max_heap) - 2 >= len(self.min_heap) - 1:
            half_max = -heapq.heappop(self.max_heap)
            heapq.heappush(self.min_heap, half_max)

        while len(self.min_heap) - 1 - 2 >= len(self.max_heap):
            half_min = heapq.heappop(self.min_heap)
            heapq.heappush(self.max_heap, -half_min)
        
        # print(self.min_heap, self.max_heap)


    def findMedian(self) -> float:
        if len(self.max_heap) == len(self.min_heap) - 1:
            return 0.5 * (-self.max_heap[0] + self.min_heap[0])
        if len(self.max_heap) > len(self.min_heap) - 1:
            return -self.max_heap[0]
        return self.min_heap[0]


# Your MedianFinder object will be instantiated and called as such:
# obj = MedianFinder()
# obj.addNum(num)
# param_2 = obj.findMedian()

if __name__ == "__main__":
    mf = MedianFinder()
    mf.addNum(40)
    assert mf.findMedian() == 40, mf.findMedian()
    mf.addNum(12)
    assert mf.findMedian() == 26
    mf.addNum(40)
    assert mf.findMedian() == 40