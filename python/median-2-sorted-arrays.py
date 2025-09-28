class Solution(object):
    # Rank 'r' is 0-based
    def find_rank(self, a, b, r):
        if len(a) == 0:
            return b[r]
        elif len(b) == 0:
            return a[r]

        na, nb = len(a), len(b)
        ma, mb = na // 2, nb // 2
        if a[ma] > b[mb]:
            a, b = b, a
            na, nb = nb, na
            ma, mb = mb, ma

        # a[ma] <= b[mb] from now on

        print("Pre transformation: ", a, b, r)
        if r < ma + mb + 1:
            b = b[0:mb]
        else:  # r >= ma + mb + 1:
            a = a[ma + 1 :]
            r -= ma + 1
        print("Post transformation: ", a, b, r)
        return self.find_rank(a, b, r)

    def findMedianSortedArrays(self, nums1, nums2):
        """
        :type nums1: List[int]
        :type nums2: List[int]
        :rtype: float
        """
        n = len(nums1) + len(nums2)
        m = n // 2
        v = self.find_rank(nums1, nums2, m)
        if m * 2 < n:
            return v
        prev_v = self.find_rank(nums1, nums2, m - 1)
        return (v + prev_v) / 2.0
