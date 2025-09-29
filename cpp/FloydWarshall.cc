
#include "../cpp/header.h"

class FloydWarshall {
 public:
  struct Edge {
    // Directed edge: a -> b
    int a, b;
    size_t w;
  };

  FloydWarshall(const size_t n, const std::vector<Edge>& edges)
    : n_(n)
    , shortestD_(n, std::vector<size_t>(n, std::numeric_limits<size_t>::max() / 3 /* avoid overflow */))
    , nextEdge_(n, std::vector<int>(n, -1))
    , edges_(edges) {
    for (size_t k = 0; k < n; ++k) {
      shortestD_[k][k] = 0;
      nextEdge_[k][k] = 0;
    }
    for (int ei = -1; auto& e : edges) {
      ++ei;
      if (shortestD_[e.a][e.b] > e.w) {
        shortestD_[e.a][e.b] = e.w;
        nextEdge_[e.a][e.b] = ei;
      }
    }

    for (size_t k = 0; k < n; ++k) for (size_t i = 0; i < n; ++i) for (size_t j = 0; j < n; ++j) {
      auto w = shortestD_[i][k] + shortestD_[k][j];
      if (shortestD_[i][j] > w) {
        shortestD_[i][j] = w;
        nextEdge_[i][j] = nextEdge_[i][k];
      }
    } 
  }

  std::optional<size_t> distance(int src, int dst) const {
    if (src == dst) {
      return 0;
    }
    return nextEdge_[src][dst] >= 0 ? shortestD_[src][dst] : std::optional<size_t>();
  }
  const std::vector<std::vector<size_t>>& allDistances() const {
    return shortestD_;
  }

  // NB: decreasing the weight of an edge is equivalant to adding a new edge with the target weight.
  void addEdge(const Edge e) {
    const int eIdx = edges_.size();
    edges_.push_back(e);
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
  std::vector<int> shortestPath(int src, int dst) const {
    assert(nextEdge_[src][dst] >= 0);
    std::vector<int> path;
    while (src != dst) {
      auto e = nextEdge_[src][dst];
      src = edges_[e].b;
      path.push_back(e);
    }
    return path;
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
  std::vector<std::vector<size_t>> shortestD_;
  // nextEdge[a][b] is the edge index from a on the shortest path from a to b.
  // < 0 means no path at all.
  std::vector<std::vector<int>> nextEdge_;
  std::vector<Edge> edges_;
};

class FloydWarshallTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(FloydWarshallTest, ManualCases) {
  const std::vector<FloydWarshall::Edge> edges = {
    FloydWarshall::Edge{0, 1, 1},
    FloydWarshall::Edge{1, 2, 2},
    FloydWarshall::Edge{2, 3, 3},
    FloydWarshall::Edge{0, 3, 10},
    FloydWarshall::Edge{3, 0, 1},
    FloydWarshall::Edge{1, 3, 8},
  };

  FloydWarshall fw(5, edges);
  EXPECT_FALSE(fw.distance(0, 4));

  EXPECT_EQ(fw.distance(0, 3), std::optional<size_t>(6));
  EXPECT_EQ(fw.shortestPath(0, 3), std::vector<int>({0, 1, 2}));

  EXPECT_EQ(fw.distance(3, 2), std::optional<size_t>(4));
  EXPECT_EQ(fw.shortestPath(3, 2), std::vector<int>({4, 0, 1}));

  fw.addEdge(FloydWarshall::Edge{1, 3, 6});
  EXPECT_EQ(fw.distance(0, 3), std::optional<size_t>(6));
  EXPECT_EQ(fw.shortestPath(0, 3), std::vector<int>({0, 1, 2}));

  fw.addEdge(FloydWarshall::Edge{1, 3, 4});
  EXPECT_EQ(fw.distance(0, 3), std::optional<size_t>(5));
  EXPECT_EQ(fw.shortestPath(0, 3), std::vector<int>({0, 7}));

}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

