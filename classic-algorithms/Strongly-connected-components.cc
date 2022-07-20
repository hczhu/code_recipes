
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

class SCC : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};


using Edge = std::pair<int, int>;

void sortEdges(int N, std::vector<Edge>& edges, std::vector<int>& edgeStart) {
  std::vector<int> cnt(N, 0);
  for (const auto& [a, b] : edges) {
    ++cnt[a];
  }
  for (int i = 1; i < cnt.size(); ++i) {
    cnt[i] += cnt[i-1];
  }
  edgeStart.resize(N + 1);
  edgeStart[0] = 0;
  for (int i = 0; i < cnt.size(); ++i) {
    edgeStart[i + 1] = cnt[i];
  }
  std::vector<Edge> sortedEdges(edges.size());
  for (const auto& [a, b] : edges) {
    CHECK_GT(cnt[a], 0);
    sortedEdges[--cnt[a]] = Edge(a, b);
  }
  edges = sortedEdges;
}

// A two pass algorithm
// 'scc' will be returned SCCs in topological order.
// I.E. if there is an edge from scc[i] to scc[j], then i < j
std::vector<std::vector<int>> KosarajuAlgorithm(int N, std::vector<Edge> edges) {
  std::vector<int> edgeStart;
  sortEdges(N, edges, edgeStart);
  CHECK_EQ(edgeStart.size(), N + 1);

  auto dfs = [&edges, &edgeStart, N] (const std::vector<int>& nodeOder, auto nodeFinishCallback) {
    // LOG(INFO) << "Running dfs.";
    std::vector<bool> visited(N, false);
    std::vector<int> stack;
    auto updatedEdgeStart = edgeStart;
    int dfsId = -1;
    for (auto root : nodeOder) {
      if (visited[root]) {
        continue;
      }
      ++dfsId;
      stack.push_back(root);
      visited[root] = true;
      while (!stack.empty()) {
        auto v = stack.back();
        auto &p = updatedEdgeStart[v];
        while (p < edgeStart[v + 1] && visited[edges[p].second]) {
          ++p;
        }
        if (p == edgeStart[v + 1]) {
          nodeFinishCallback(v, dfsId);
          stack.pop_back();
        } else {
          auto next = edges[p].second;
          stack.push_back(next);
          visited[next] = true;
        }
      }
    }
  };

  std::vector<int> nodeOrder(N);
  for (int i = 0; i < N; ++i) {
    nodeOrder[i] = i;
  }
  std::vector<int> dfsOrder;
  auto firstPassCallback = [&dfsOrder](int v, int dfsId) {
    dfsOrder.push_back(v);
  };

  dfs(nodeOrder, firstPassCallback);

  for (auto& e : edges) {
    std::swap(e.first, e.second);
  }
  sortEdges(N, edges, edgeStart);

  // EXPECT_EQ(edges, std::vector<Edge>());
  std::vector<std::vector<int>> scc;
  auto secondPassCallback = [&scc](int v, int dfsId) {
    if (scc.size() == dfsId) {
      scc.emplace_back();
    }
    scc[dfsId].push_back(v);
  };
  reverse(dfsOrder.begin(), dfsOrder.end());
  // EXPECT_EQ(dfsOrder, std::vector<int>());
  dfs(dfsOrder, secondPassCallback);

  return scc;
}

// 'scc' will be returned SCCs in topological order.
// I.E. if there is an edge from scc[i] to scc[j], then i < j
std::vector<std::vector<int>> onePassAlgorithm(int N, std::vector<Edge> edges) {
  std::vector<int> edgeStart;
  sortEdges(N, edges, edgeStart);
  std::vector<int> movingEdgeStart = edgeStart;

  // Already assigned to an SCC
  constexpr int kInAnotherScc = -1;
  constexpr int kUnvisited = 0;
  
  std::vector<int> stack;
  std::vector<bool> visited(N, false);
  std::vector<int> flags(N, kUnvisited);
  std::vector<int> low(N, 0);
  int gOrder = 0;
  std::vector<std::vector<int>> scc;
  std::vector<int> firstSeenOrder;
  auto pushToStack = [&](int v) {
    stack.push_back(v);
    gOrder++;
    flags[v] = low[v] = gOrder;
    firstSeenOrder.push_back(v);
  };
  for (int root = 0; root < N; ++root) {
    if (flags[root] != kUnvisited) {
      continue;
    }
    pushToStack(root);
    // LOG(INFO) << "Root " << root << " low = " << low[root];
    while (!stack.empty()) {
      auto v = stack.back();
      auto& p = movingEdgeStart[v];
      int u = -1;
      while (p < edgeStart[v + 1] && kUnvisited != flags[u = edges[p].second]) {
        ++p;
        if (flags[u] != kInAnotherScc) {
          // LOG(INFO) << v << " ----> " << u;
          low[v] = std::min(low[v], low[u]);
        }
      }
      if (p == edgeStart[v + 1]) {
        stack.pop_back();
        // LOG(INFO) << "Pop out low[" << v << "] = " << low[v] << " with flag " << flags[v];
        if (!stack.empty()) {
          auto parent = stack.back();
          low[parent] = std::min(low[parent], low[v]);
        }
        if (low[v] == flags[v]) {
          scc.emplace_back();
          int lastNode = -1;
          do {
            lastNode = firstSeenOrder.back();
            scc.back().push_back(lastNode);
            firstSeenOrder.pop_back();
            flags[lastNode] = kInAnotherScc;
          } while(lastNode != v);
        }
      } else {
        pushToStack(u);
        ++p;
        // LOG(INFO) << "New node " << u << " low = " << low[u];
      }
    }
  }
  std::reverse(scc.begin(), scc.end());
  return scc;
}

size_t solve(int N, int M, std::vector<int> A, std::vector<int> B) {
  std::vector<Edge> edges(M);
  std::vector<std::vector<int>> node2Edges(N);
  for (int i = 0; i < M; ++i) {
    edges[i] = Edge(A[i] - 1, B[i] - 1);
    node2Edges[edges[i].first].push_back(edges[i].second);
  }
  
  auto scc = onePassAlgorithm(N, edges);
  std::vector<int> node2scc(N);
  std::reverse(scc.begin(), scc.end());
  for (int i = 0; i < scc.size(); ++i) {
    for (auto v : scc[i]) {
      node2scc[v] = i;
    }
  }

  std::vector<size_t> maxVisits(scc.size(), 0);
  for (int i = 0; i < scc.size(); ++i) {
    maxVisits[i] = scc[i].size();
    for (auto v : scc[i]) {
      auto v_scc = node2scc[v];
      for (auto u : node2Edges[v]) {
        auto u_scc = node2scc[u];
        if (v_scc != u_scc) {
          CHECK_GT(v_scc, u_scc);
          maxVisits[v_scc] = std::max(maxVisits[v_scc], scc[i].size() + maxVisits[u_scc]);
        }
      }
    }
  }
  return *std::max_element(maxVisits.begin(), maxVisits.end());
}

TEST_F(SCC, BasicTest) {
  std::vector<int> A = {
   3, 2, 5, 9, 10, 3, 3, 9, 4,
  };

  std::vector<int> B = {
    9, 5, 7, 8, 6, 4, 5, 3, 9,
  };

  std::vector<Edge> edges;
  for (int i = 0; i < A.size(); ++i) {
    edges.emplace_back(A[i] - 1, B[i] - 1);
  }
  auto edgesCopy = edges;
  {
    std::vector<std::vector<int>> expectedScc = {{9}, {5}, {3, 8, 2}, {7},
                                                 {1}, {4}, {6},       {0}};
    EXPECT_EQ(KosarajuAlgorithm(10, edges), expectedScc);
  }

  {
    EXPECT_EQ(edges, edgesCopy);
    std::vector<std::vector<int>> expectedScc = {
      { 9 }, { 5 }, { 8, 3, 2 }, { 7 }, { 1 }, { 4 }, { 6 }, { 0 }
    };
    EXPECT_EQ(onePassAlgorithm(10, edges), expectedScc);
  }

  EXPECT_EQ(solve(10, A.size(), A, B), 5);
}

TEST_F(SCC, randomTest) {
  auto testOne = [] {
    std::vector<Edge> edges;
    int N = 12;
    int M = 30;
    for (int i = 0; i < M; ++i) {
      edges.emplace_back(rand() % N, rand() % N);
    }
    // EXPECT_EQ(std::vector<Edge>(), edges);
    auto scc1 = KosarajuAlgorithm(N, edges);
    auto scc2 = onePassAlgorithm(N, edges);
    auto sortScc = [](auto& scc) {
      for (auto& s : scc) {
        std::sort(s.begin(), s.end());
      }
      std::sort(scc.begin(), scc.end());
    };
    sortScc(scc1);
    sortScc(scc2);
    EXPECT_EQ(scc1, scc2);
  };
  for (int i = 0; i < 100000; ++i) {
    testOne();
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

