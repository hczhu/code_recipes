from collections import defaultdict
from lib2to3.pytree import Node
from netrc import netrc
from re import I
from sys import flags
from urllib.parse import scheme_chars
from xmlrpc.client import Boolean


def findSCC(edges):
    """
    return scc
    scc[i] contains all nodes in the i-th SCC.
    """
    N = len(edges)
    edge_p = [0] * N
    kUnvisited = -1
    kAssignedScc = -2
    # Candidates for a SCC
    first_seen_order = []
    flags = [kUnvisited] * N
    low = [0] * N
    gorder = 1


    stack = []

    # SCCs will be sorted in topological order
    # I.E. if there is an edge from scc[a] to scc[b], then a > b 
    scc = []

    def push_node(v):
        stack.append(v)
        first_seen_order.append(v)

        nonlocal gorder
        gorder += 1
        flags[v] = low[v] = gorder

    def dfs(root):
        push_node(root)
        while len(stack) > 0:
            v = stack[-1]
            while edge_p[v] < len(edges[v]) and kUnvisited != flags[edges[v][edge_p[v]]]:
                next = edges[v][edge_p[v]]
                edge_p[v] += 1
                if flags[next] != kAssignedScc:
                    low[v] = min(low[v], low[next])

            if edge_p[v] == len(edges[v]):
                stack.pop()
                if len(stack) > 0:
                    low[stack[-1]] = min(low[stack[-1]], low[v])
                if flags[v] == low[v]:
                    node = -1
                    new_scc = []
                    while node != v:
                        node = first_seen_order[-1]
                        first_seen_order.pop()
                        new_scc.append(node)
                        flags[node] = kAssignedScc
                    scc.append(new_scc)
                continue

            next = edges[v][edge_p[v]]
            edge_p[v] += 1
            push_node(next)


    for v in range(N):
        if kUnvisited == flags[v]:
            dfs(v)
    return scc


from typing import List
# Write any import statements here

def getMaxVisitableWebpages(N: int, M: int, A: List[int], B: List[int]) -> int:
  # Write your code here
  edges = [[] for _ in range(N)]
  for a, b in zip(A, B):
    edges[a - 1].append(b - 1)

  
  # print(edges)
  scc = findSCC(edges)
  print(scc)
  print("========")
  max_visits = [len(s) for s in scc]
  node2scc = {
    s: i
    for i in range(len(scc))
        for s in scc[i]
  }
  for s in scc:
    for v in s:
        for u in edges[v]:
            scc_v = node2scc[v]
            scc_u = node2scc[u]
            if scc_v != scc_u:
                max_visits[scc_v] = max(
                    max_visits[scc_v],
                    max_visits[scc_u] + len(s)
                )
  return max(max_visits)

print(getMaxVisitableWebpages(N = 4,
M = 4,
A = [1, 2, 3, 4],
B = [4, 1, 2, 1],
)
)

print(getMaxVisitableWebpages(
    N = 5,
M = 6,
A = [3, 5, 3, 1, 3, 2],
B = [2, 1, 2, 4, 5, 4],
)
)
print(getMaxVisitableWebpages(
N = 10,
M = 9,
A = [3, 2, 5, 9, 10, 3, 3, 9, 4, 4],
B = [9, 5, 7, 8, 6, 4, 5, 3, 9, 4],
))