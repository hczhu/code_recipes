#include "../header.h"


class SCC : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};


using DEL = graph::DirectEdgeList<>;
// A two pass algorithm
// 'scc' will be returned SCCs in topological order.
// I.E. if there is an edge from scc[i] to scc[j], then i < j
std::vector<std::vector<int>> KosarajuAlgorithm(int n, DEL &el) {
  auto dfs = [&el, n](const std::vector<int> &nodeOder,
                      auto nodeFinishCallback) {
    // LOG(INFO) << "Running dfs.";
    std::vector<bool> visited(n, false);
    std::vector<int> stack;
    std::vector<DEL::Iterator> edgeItr;
    for (size_t i = 0; i < n; ++i) {
      edgeItr.push_back(el.edgeIterator(i));
    }
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
        auto &itr = edgeItr[v];
        while (itr.valid() && visited[itr.tail()]) {
          ++itr;
        }
        if (!itr.valid()) {
          nodeFinishCallback(v, dfsId);
          stack.pop_back();
        } else {
          auto next = itr.tail();
          ++itr;
          stack.push_back(next);
          visited[next] = true;
        }
      }
    }
  };

  std::vector<int> nodeOrder(n);
  for (int i = 0; i < n; ++i) {
    nodeOrder[i] = i;
  }
  std::vector<int> dfsOrder;
  auto firstPassCallback = [&dfsOrder](int v, int dfsId) {
    dfsOrder.push_back(v);
  };

  dfs(nodeOrder, firstPassCallback);

  el.reverse();

  // EXPECT_EQ(edges, std::vector<Edge>());
  std::vector<std::vector<int>> scc;
  auto secondPassCallback = [&scc](int v, int dfsId) {
    if (scc.size() == dfsId) {
      scc.emplace_back();
    }
    scc[dfsId].push_back(v);
  };
  std::reverse(dfsOrder.begin(), dfsOrder.end());
  // EXPECT_EQ(dfsOrder, std::vector<int>());
  dfs(dfsOrder, secondPassCallback);
  el.reverse();

  return scc;
}

// 'scc' will be returned SCCs in topological order.
// I.E. if there is an edge from scc[i] to scc[j], then i < j
std::vector<std::vector<int>> onePassAlgorithm(int n, DEL& el) {
  // Already assigned to an SCC
  constexpr int kInAnotherScc = -1;
  constexpr int kUnvisited = 0;
  std::vector<DEL::Iterator> edgeItr;
  for (size_t i = 0; i < n; ++i) {
    edgeItr.push_back(el.edgeIterator(i));
  }

  std::vector<int> stack;
  std::vector<bool> visited(n, false);
  std::vector<int> flags(n, kUnvisited);
  std::vector<int> low(n, 0);
  int gOrder = 0;
  std::vector<std::vector<int>> scc;
  std::vector<int> firstSeenOrder;
  auto pushToStack = [&](int v) {
    stack.push_back(v);
    gOrder++;
    flags[v] = low[v] = gOrder;
    firstSeenOrder.push_back(v);
  };
  for (int root = 0; root < n; ++root) {
    if (flags[root] != kUnvisited) {
      continue;
    }
    pushToStack(root);
    // LOG(INFO) << "Root " << root << " low = " << low[root];
    while (!stack.empty()) {
      auto v = stack.back();
      auto& itr = edgeItr[v];
      size_t u = n;
      while (itr.valid() && kUnvisited != flags[u = itr.tail()]) {
        ++itr;
        if (flags[u] != kInAnotherScc) {
          // LOG(INFO) << v << " ----> " << u;
          low[v] = std::min(low[v], low[u]);
        }
      }
      if (itr.valid()) {
        pushToStack(u);
        ++itr;
        // LOG(INFO) << "New node " << u << " low = " << low[u];
      } else {
        stack.pop_back();
        // LOG(INFO) << "Pop out low[" << v << "] = " << low[v] << " with flag "
        // << flags[v];
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
          } while (lastNode != v);
        }
      }
    }
  }
  std::reverse(scc.begin(), scc.end());
  return scc;
}

size_t solve(int N, int M, std::vector<int> A, std::vector<int> B) {
  std::vector<std::vector<int>> node2Edges(N);
  DEL el;
  for (int i = 0; i < M; ++i) {
    el.addEdge(A[i] - 1, B[i] - 1);
    node2Edges[A[i] - 1].push_back(B[i] - 1);
  }

  el.finalize();
  
  auto scc = onePassAlgorithm(N, el);
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

  DEL el;
  for (int i = 0; i < A.size(); ++i) {
    el.addEdge(A[i] - 1, B[i] - 1);
  }
  {
    std::vector<std::vector<int>> expectedScc = {{9}, {5}, {3, 8, 2}, {7},
                                                 {1}, {4}, {6},       {0}};
    el.finalize();
    EXPECT_EQ(KosarajuAlgorithm(10, el), expectedScc);
  }

  {
    std::vector<std::vector<int>> expectedScc = {{9}, {5}, {8, 3, 2}, {7},
                                                 {1}, {4}, {6},       {0}};
    el.finalize();
    EXPECT_EQ(onePassAlgorithm(10, el), expectedScc);
  }

  EXPECT_EQ(solve(10, A.size(), A, B), 5);
}

TEST_F(SCC, randomTest) {
  auto testOne = [] {
    DEL el;
    int N = 12;
    int M = 30;
    for (int i = 0; i < M; ++i) {
      el.addEdge(rand() % N, rand() % N);
    }
    // EXPECT_EQ(std::vector<Edge>(), edges);
    el.finalize();
    auto scc1 = KosarajuAlgorithm(N, el);
    el.finalize();
    auto scc2 = onePassAlgorithm(N, el);
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

