from dataclasses import dataclass
from typing import List


@dataclass
class TreeNode:
    idx: int
    l_child: object = None
    r_child: object = None


def getCartesianTree(elements):
    """
    https://www.wikiwand.com/en/Cartesian_tree
    return a Cartesian tree with tree nodes as TreeNode
    """
    # Store the right most tree path. The top of the stack is the
    # lowest tree node. Therefore, the nodes are sorted in the stack
    # with the largest node on the top.
    stack = []
    for i in range(len(elements)):
        new_node = TreeNode(idx=i)
        while len(stack) > 0 and elements[stack[-1].idx] >= elements[i]:
            new_node.l_child = stack[-1]
            stack.pop()
        if len(stack) > 0:
            stack[-1].r_child = new_node
        stack.append(new_node)

    return stack[0]


# print(getCartesianTree([9, 3, 7, 1, 8,]))


M = 10 ** 9 + 7


class Solution:
    def solve(self, root, b, e):
        res = 0
        while root is not None:
            m = root.idx
            assert m >= b and m < e, (b, e, m, root)
            # print("-=======")
            s = (
                (
                    self.s2r[b]
                    + M
                    - self.s2r[m]
                    + M
                    - (((m - b) * (self.sr[m])) % M)
                )
                * (e - m)
            ) % M
            # print(s)
            s += (
                (
                    self.s2[e]
                    + M
                    - self.s2[m + 1]
                    + M
                    - (((e - 1 - m) * self.s[m + 1]) % M)
                )
                * (m - b + 1)
            ) % M
            # print(s)
            s += (((self.sh[m] * (m + 1 - b)) % M) * (e - m)) % M
            # print(s)
            s = (s * self.sh[m]) % M
            # print(b, e, m, " = ", s)

            res = (res + s) % M

            b1, e1 = b, m
            b2, e2 = m + 1, e
            child = root.r_child
            new_root = root.l_child
            if e1 - b1 < e2 - b2:
                b1, b2 = b2, b1
                e1, e2 = e2, e1
                child = root.l_child
                new_root = root.r_child
            res = (res + self.solve(child, b2, e2)) % M
            b, e = b1, e1
            root = new_root
        return res

    def totalStrength(self, sh: List[int]) -> int:
        self.sh = sh
        self.s = [0] * (len(sh) + 1)
        self.s2 = [0] * (len(sh) + 1)
        for i in range(len(sh)):
            self.s[i + 1] = (self.s[i] + sh[i]) % M
            self.s2[i + 1] = (self.s2[i] + self.s[i + 1]) % M

        self.sr = [0] * (len(sh) + 1)
        self.s2r = [0] * (len(sh) + 1)
        for i in range(len(sh) - 1, -1, -1):
            self.sr[i] = (self.sr[i + 1] + self.sh[i]) % M
            self.s2r[i] = (self.s2r[i + 1] + self.sr[i]) % M

        # print(self.s, self.s2, self.sr, self.s2r)
        root = getCartesianTree(sh)
        return self.solve(root, 0, len(sh))


sol = Solution()

assert sol.totalStrength([1, 3, 1, 2]) == 44

assert sol.totalStrength([5, 4, 6]) == 213
