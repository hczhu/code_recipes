class Solution(object):
    def isMatch(self, s, p):
        """
        :type s: str
        :type p: str
        :rtype: bool
        """
        
        # p[:i] match s[:j]?
        m = [False] * (1 + len(s))
        m[0] = True
        for pc in p:
            nm = [False] * (1 + len(s))
            nm[0] = m[0] and pc == '*'
            has_match = m[0]
            for j in range(1, len(nm)):
                if pc == '?':
                    nm[j] = m[j - 1]
                elif pc == '*':
                    has_match = has_match or m[j]
                    nm[j] = has_match
                else:  # pc in 'a-z'
                    nm[j] = m[j - 1] and pc == s[j-1]
            m = nm
        return m[-1]
        
