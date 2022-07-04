class Solution:
    def subStrHash(self, s: str, power: int, modulo: int, k: int, hashValue: int) -> str:
        s = s[::-1]
        p, m = power, modulo
        pk = 1
        for _ in range(k):
            pk = (pk * p) % m
        
        h = 0
        def idx(ch):
            return ord(ch) -ord("a") + 1
        for ch in s[0:k]:
            h = (h * p + idx(ch)) % m
        
        res = ""
        if h == hashValue:
            res = s[0:k][::-1]
        # print(s[0:k], h)

        for i in range(k, len(s)):
            h = (h * p + idx(s[i])) % m
            h = (h + m - (idx(s[i-k]) * pk) %m) %m
            # print(s[i-k+1:i+1], h)
            if h == hashValue:
                res = s[i-k+1:i+1][::-1]
        
        # print(res)
        return res



sol = Solution()
assert sol.subStrHash(
    s = "leetcode", power = 7, modulo = 20, k = 2, hashValue = 0
) == "ee"

assert sol.subStrHash(
    s = "fbxzaad", power = 31, modulo = 100, k = 3, hashValue = 32
) == "fbx"