# https://leetcode.com/problems/trapping-rain-water/submissions/
class Solution(object):
    def trap(self, height):
        """
        :type height: List[int]
        :rtype: int
        """
        # height & width
        water_levels = []
        res = 0
        for h in height:
            w = 0
            while len(water_levels) > 0 and water_levels[-1][0] <= h:
                if len(water_levels) > 1:
                    w += water_levels[-1][1]
                    res += w * (min(h, water_levels[-2][0]) - water_levels[-1][0])
                water_levels.pop()
            if len(water_levels) == 0:
                water_levels.append((h, 1))
            else:
                water_levels.append((h, w + 1))
            # print(water_levels, "=", res)
        return res
            
            
        