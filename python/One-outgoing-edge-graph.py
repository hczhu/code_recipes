from ssl import _create_default_https_context
from typing import List
# Write any import statements here

def getMaxVisitableWebpages(N: int, L: List[int]) -> int:
  # Write your code here
  incoming_edges = [0] * N
  visited = [False] * N
  
  for e in L:
    incoming_edges[e - 1] += 1

  stack = [
    v for v in range(N) if incoming_edges[v] == 0
  ]

  topolocal_order = []
  while len(stack) > 0:
    v = stack[-1]
    topolocal_order.append(v)
    stack.pop()
    next = L[v] - 1
    visited[v] = True
    incoming_edges[next] -= 1
    if incoming_edges[next] == 0:
      stack.append(next)
      visited[next] = True
  
  # The unvisited nodes are cycles only
  max_visits = {}
  for v in range(N):
    if not visited[v]:
      cycle = []
      while not visited[v]:
        visited[v] = True
        cycle.append(v)
        v = L[v] - 1
      print(cycle)
      for v in cycle:
        max_visits[v] = len(cycle)
  
  print(topolocal_order)
  for i in range(len(topolocal_order) - 1, -1, -1):
    v = topolocal_order[i]
    next = L[v] - 1
    assert next in max_visits
    max_visits[v] = max_visits[next] + 1
  
  print(max_visits)
  return max(max_visits.values())

  
print(getMaxVisitableWebpages(5,  [2, 4, 2, 2, 3]))