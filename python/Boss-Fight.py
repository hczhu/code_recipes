from typing import List
# Write any import statements here




def getMaxDamageDealt(N: int, H: List[int], D: List[int], B: int) -> float:
  # Write your code here
  res = 0
  for a in range(N):
    for b in range(N):
      if a == b: continue
      res = max(res, H[a] * D[a] + H[b] * D[b] + H[a] * D[b])
  return res / B

