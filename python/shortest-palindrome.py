class Solution(object):
    def getKMP(self, s):
        """
        return the KMP array of 's'
        """
        kmp = [0] * len(s)
        prev = 0
        # s[ : kmp[i]] == s[i - kmp[i] + 1: i + 1] and kmp[i] < i + 1
        kmp[0] = 0
        for i in range(1, len(s)):
            while prev > 0 and s[prev] != s[i]:
                prev = kmp[prev - 1]
            if s[prev] == s[i]:
                prev += 1
            kmp[i] = prev
        return kmp
        
    def shortestPalindrome(self, s):
        """
        :type s: str
        :rtype: str
        """
        if len(s) == 0: return ""
        
        kmp = self.getKMP(s)
        # print(kmp)
        
        p = 0
        for i in range(len(s) - 1, -1, -1):
            c = s[i]
            while p > 0 and c != s[p]:
                p = kmp[p - 1]
            if c == s[p]:
                p += 1
        
        # print("p = ", p)
        
        return s[p:][::-1] + s
            
            
        
