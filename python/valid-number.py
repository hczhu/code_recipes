class Solution(object):
    def removeSign(self, s):
        if len(s) == 0: return s
        if s[0] in "+-": s = s[1:]
        return s
        
        
    def isNumber(self, s):
        """
        :type s: str
        :rtype: bool
        """
        
        s = self.removeSign(s)
        s = s.lower()
        
        p = s.find("e")
        if p == 0 or p + 1 == len(s):
            return False
        
        # "".isnumeric() == False
        
        if p > 0:
            if not self.removeSign(s[p+1:]).isnumeric():
                return False
            s = s[:p]            
            
        if s == ".": return False
        
        p = s.find(".")
        if p < 0: return s.isnumeric()
        
        return (p == 0 or s[:p].isnumeric()) and (p+1 == len(s) or s[p+1:].isnumeric())
