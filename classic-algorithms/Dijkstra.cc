
#include "../cpp/header.h"

class Dijkstra {
 public:
  struct Edge {
    int out;
    size_t w;
  };
  Dijkstra(size_t n) : n_(n), el_(n) {}

  void addEdge(int a, int b, size_t w) {
    CHECK_LT(a, n_);
    CHECK_LT(b, n_);
    el_[a].push_back(Edge{b, w});
  }

  void run(std::vector<int> srcs, std::vector<size_t>& dist) {
    // Assume dist is already initialized correctly, e.g., dist[src] shouldn't
    // be the upper limit.
    CHECK_EQ(dist.size(), n_);
    using DistAndNode = std::pair<size_t, int>;
    auto cmp = std::greater<DistAndNode>();
    std::vector<DistAndNode> heap;
    for (auto src : srcs) {
      heap.emplace_back(dist[src], src);
    }
    while (!heap.empty()) {
      std::pop_heap(heap.begin(), heap.end(), cmp);
      auto [d, v] = heap.back();
      heap.pop_back();
      if (d > dist[v]) {
        continue;
      }
      for (auto& e : el_[v]) {
        auto dd = d + e.w;
        if (dd < dist[e.out]) {
          dist[e.out] = dd;
          heap.emplace_back(dd, e.out);
          std::push_heap(heap.begin(), heap.end(), cmp);
        }
      }
    }
  }

 private:
  size_t n_;
  // el_[i] is the list of edges from node i to any other nodes.
  std::vector<std::vector<Edge>> el_;
};

class DijkstraTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DijkstraTest, ManualCases) {
  Dijkstra d(5);
  d.addEdge(0, 1, 1);
  d.addEdge(1, 2, 2);
  d.addEdge(2, 3, 3);
  d.addEdge(0, 3, 10);
  d.addEdge(3, 0, 1);
  d.addEdge(1, 3, 8);
  constexpr size_t kInf = std::numeric_limits<size_t>::max();
  std::vector<size_t> dist0(5, kInf);
  dist0[0] = 0;
  std::vector<size_t> dist3(5, kInf);
  dist3[3] = 0;

  d.run({0}, dist0);
  // 0 --> 1 --> 2 --> 3
  EXPECT_EQ(dist0[3], 6);
  EXPECT_EQ(dist0[4], kInf);

  d.run({3}, dist3);
  EXPECT_EQ(dist3[2], 4);

  d.addEdge(1, 3, 6);
  d.run({1}, dist0);
  EXPECT_EQ(dist0[3], 6);

  d.addEdge(1, 3, 4);
  d.run({1}, dist0);
  EXPECT_EQ(dist0[3], 5);

  d.run({1}, dist3);
  EXPECT_EQ(dist3[2], 4);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

