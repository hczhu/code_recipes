
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits.h>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <valarray>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../data-structures/Graph.h"

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

#define _STR(x) #x
#define STR(x) _STR(x)
#define PRINT_MACRO(M) static_assert(0, "Macro " #M " = " STR(M))

// #define x 42
// PRINT_MACRO(x);

/* template end */

using DEL = graph::DirectEdgeList<>;

// 'nl': the number of vertexes on the left side of the bipartite graph.
// 'nr': the number of vertexes on the right side of the bipartite graph.
std::vector<size_t> maximumCardinalityBipartiteMatch(size_t nl, size_t nr,
                                                     DEL &el) {
  std::vector<size_t> rightVertexFlags(nr, 0);
  // 'rightMatches[i]' means right side vertex i matches left side vertex
  // 'rightMatches[i]'.
  std::vector<size_t> rightMatches(nr, nl);
  size_t currentFlag = 1;

  auto recurseAugment = [&](size_t l, auto &&recurseAugment) -> bool {
    for (auto itr = el.edgeIterator(l); itr.valid(); ++itr) {
      int r = itr.tail();
      if (rightVertexFlags[r] == currentFlag) {
        // Already visited this vertex in this traverse.
        continue;
      }
      rightVertexFlags[r] = currentFlag;
      if (rightMatches[r] == nl ||
          recurseAugment(rightMatches[r], recurseAugment)) {
        rightMatches[r] = l;
        return true;
      }
    }
    return false;
  };

  auto runAugment = [&](size_t l) { return recurseAugment(l, recurseAugment); };

  for (size_t l = 0; l < nl; l++) {
    if (runAugment(l)) {
      ++currentFlag;
    }
  }
  return rightMatches;
}

TEST(CadinalityMatchTest, Basic) {
  auto testOne = [](std::vector<int> heads, std::vector<int> tails) {
    DEL el;
    for (int i = 0; i < heads.size(); ++i) {
      el.addEdge(heads[i], tails[i]);
    }
    el.finalize();
    const size_t nl = 1 + *std::max_element(heads.begin(), heads.end());
    const size_t nr = 1 + *std::max_element(tails.begin(), tails.end());
    auto rmatches = maximumCardinalityBipartiteMatch(nl, nr, el);
    return std::count_if(rmatches.begin(), rmatches.end(),
                         [&](auto v) { return v < nl; });
  };

  EXPECT_EQ(3, testOne(
                   {
                       0,
                       0,
                       1,
                       1,
                       2,
                       2,
                   },
                   {
                       3,
                       1,
                       2,
                       3,
                       3,
                       0,
                   }));

  EXPECT_EQ(2, testOne( {0, 1, 2, }, {3, 3, 0 }));
}

TEST(RecursiveLambda, Basic) {
  auto fib = [&](size_t n, auto &&fib) -> size_t {
    if (n <= 1) {
      return n;
    }
    return fib(n - 1, fib) + fib(n - 2, fib);
  };
  auto runFib = [&](size_t n) {
    return fib(n, fib);
  };
  EXPECT_EQ(8, runFib(6));
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

