#include <iostream>
using namespace std;
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>
#include <optional>
#include <cassert>
#include <algorithm>
using namespace std;

class Dijkstra {
 public:
  struct Edge {
    int out;
    size_t w;
  };
  Dijkstra(size_t n) : n_(n), el_(n) {}

  void addEdge(int a, int b, size_t w) {
    el_[a].push_back(Edge{b, w});
  }

  void run(std::vector<int> srcs, std::vector<size_t>& dist) {
    // Assume dist is already initialized correctly, e.g., dist[src] shouldn't
    // be the upper limit.
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

class Solution {
public:
    vector<vector<int>> modifiedGraphEdges(int n, vector<vector<int>> edges, int src, int dst, int _target) {
      const size_t target = _target;
      Dijkstra dj{size_t(n)};

      for (int ei=-1; const auto& e : edges) {
        ++ei;
        size_t w = e[2] < 0 ? target : e[2];
        dj.addEdge(e[0], e[1], w);
        dj.addEdge(e[1], e[0], w);
      }
      constexpr size_t kInf = std::numeric_limits<size_t>::max() / 2;
      std::vector<size_t> dSrc(n, kInf), dDst(n, kInf);
      dSrc[src] = dDst[dst] = 0;
      dj.run({src}, dSrc);
      dj.run({dst}, dDst);
      if (dSrc[dst] == kInf) {
        cout << "Src and Dst are not connected";
        return {};
      }
      if (dSrc[dst] < target) {
        cout << "Distance is too short already: " << dSrc[dst] << " vs. " << target << endl;
        return {};
      }

      for (auto& e : edges) {
        if (e[2] < 0) {
          size_t w = 1;
          auto s = min(
            dSrc[e[0]] + w + dDst[e[1]],
            dSrc[e[1]] + w + dDst[e[0]]
          );
          if (s < target) {
            w += target - s;
          }
          e[2] = w;
          cout << "Adding an edge " << e[0] << " <--> " << e[1] << " = " << w << " lower bound: " << s << endl;
          dj.addEdge(e[0], e[1], w);
          dj.addEdge(e[1], e[0], w);
          dj.run({e[0], e[1]}, dSrc);
          dj.run({e[0], e[1]}, dDst);
        }
      }
      if (dSrc[dst] == target) {
        return edges;
      }
      cout << "Distance is still too long: " << dSrc[dst] << " vs. " << target << endl;
      return {};
    }
};
