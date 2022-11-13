from curses.ascii import isalpha


class Solution(object):
    Cache = {}
    def Return(self, k, v):
        self.Cache[k] = v
        return v

    def isMatch(self, s, p):
        """
        :type s: str
        :type p: str
        :rtype: bool
        """
        if len(s) == 0 or len(p) == 0:
            return len(s) == 0 and len(p) == 0

        k = (len(s), len(p))
        if k in self.Cache:
            return self.Cache[k]
        
        if p[-1].isalpha():
            if p[-1] != s[-1]:
                return self.Return(k, False)
            return self.Return(k, self.isMatch(s[:-1], p[:-1]))

        if p[-1] == '.':
           return self.Return(k, self.isMatch(s[:-1], p[:-1]))

        # p[-1] == '*'
        if p[-2] == '.':
            for i in range(0, len(s) + 1):
                if self.isMatch(s[:i], p[:-2]):
                    return self.Return(k, True)
        else: # p[-2] is 'a-z'
            if self.isMatch(s, p[:-2]):
                return self.Return(k, True)
            for i in range(len(s), 0, -1):
                if s[i - 1] != p[-2]: break
                if self.isMatch(s[:i-1], p[:-2]):
                        return self.Return(k, True)
        return self.Return(k, False)