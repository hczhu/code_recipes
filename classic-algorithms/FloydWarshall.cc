
#include "../header.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>
#include <optional>
#include <cassert>
using namespace std;


class FloydWarshall {
 public:
  struct Edge {
    // Directed edge: a -> b
    int a, b;
    size_t w;
  };

  FloydWarshall(const size_t n, const vector<Edge>& edges)
    : n_(n)
    , shortestD_(n, vector<size_t>(n, numeric_limits<size_t>::max()/3))
    , nextEdge_(n, vector<int>(n, -1)) {
    
    for (size_t k = 0; k < n; ++k) {
      shortestD_[k][k] = 0;
      nextEdge_[k][k] = 0;
    }
    for (int ei = 0; auto& e : edges) {
      if (shortestD_[e.a][e.b] > e.w) {
        shortestD_[e.a][e.b] = e.w;
        nextEdge_[e.a][e.b] = ei;
      }
      ++ei;
    }

    for (size_t k = 0; k < n; ++k) for (size_t i = 0; i < n; ++i) for (size_t j = 0; j < n; ++j) {
      auto w = shortestD_[i][k] + shortestD_[k][j];
      if (shortestD_[i][j] > w) {
        shortestD_[i][j] = w;
        nextEdge_[i][j] = nextEdge_[i][k];
      }
    } 
  }

  optional<size_t> distance(int src, int dst) const {
    if (src == dst) {
      return 0;
    }
    return nextEdge_[src][dst] >= 0 ? shortestD_[src][dst] : optional<size_t>();
  }
  const vector<vector<size_t>>& allDistances() const {
    return shortestD_;
  }

  // decreasing the weight of an edge is equivilant to adding a new edge with the target weight.
  void addEdge(const Edge e, int eIdx) {
    if (shortestD_[e.a][e.b] <= e.w) {
      return;
    }
    for (size_t i = 0; i < n_; ++i) for (size_t j = 0; j < n_; ++j) {
      auto w = shortestD_[i][e.a] + e.w + shortestD_[e.b][j];
      if (shortestD_[i][j] > w) {
        shortestD_[i][j] = w;
        nextEdge_[i][j] = i == size_t(e.a) ? eIdx : nextEdge_[i][e.a];
      }
    }
  }
  void addEdgeSrc(const Edge e, int src) {
      for (int j = 0; j < n_; ++j) {
          auto w = shortestD_[src][e.a] + e.w + shortestD_[e.b][j];
          if (w < shortestD_[src][j]) {
              shortestD_[src][j] = shortestD_[j][src] = w;
          }
      }
  }

  void printMatrix() const {
    for(const auto& row : shortestD_) {
      for (const auto c : row) {
        std::cout << c << " ";
      }
      std::cout << std::endl;
    }
  }

 private:
  const size_t n_;
  vector<vector<size_t>> shortestD_;
  // nextEdge[a][b] is the edge index from a on the shortest path from a to b.
  // < 0 means no path at all.
  vector<vector<int>> nextEdge_;
};



class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(FooTest, Bar) {
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

