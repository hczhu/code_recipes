from typing import List
# Write any import statements here

def getMinimumDeflatedDiscCount(N: int, R: List[int]) -> int:
  # Write your code here
  res = 0
  prev_r = 1e12
  for i in range(N - 1, -1, -1):
    if R[i] >= prev_r:
        R[i] = i + 1
        if R[i] >= prev_r:
            return -1
        res += 1
    prev_r = R[i]
  return res


def getMinimumSecondsRequired(N: int, R: List[int], A: int, B: int) -> int:
  # Write your code here

  def get_seconds(v, target):
    if target > v:
        return A * (target - v)
    return B * (v - target)

  cache = {}
  def doit(prev_r, pos):
    if pos == N:
        return 0
    key = (prev_r, pos)
    if key in cache:
        return cache[key]
    
    res = 1e20
    r = R[pos]
    
    res = min(res, doit(prev_r + 1, pos + 1) + get_seconds(r, prev_r + 1))

    for p in range(pos, N):
        rr = R[p] - (p - pos)
        if rr > prev_r:
            res = min(res, doit(rr, pos + 1) + get_seconds(r, rr))

    cache[key] = res
    return res

  return doit(0, 0)

print(getMinimumSecondsRequired(
    N = 5,
R = [2, 5, 3, 6, 5],
A = 1,
B = 1,
  ))