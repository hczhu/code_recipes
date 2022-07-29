#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>


class LazyArray {
 public:
  using MapFn = std::function<int(int)>;
  LazyArray() = default;
  LazyArray(std::vector<int> values) : values_(std::move(values)) {}

  LazyArray map(MapFn f) {
    LazyArray child;
    child.fn_ = std::move(f);
    child.parent_ = this;
    return child;
  }

  // std::string::npos maximum size_t
  size_t indexOf(int target) {
    std::vector<MapFn> fns;
    auto p = this;
    const std::vector<int>* pv = nullptr;
    while (p != nullptr) {
      pv = &p->values_;
      if (p->fn_) {
        fns.push_back(p->fn_);
      }
      p = p->parent_;
    }
    std::reverse(fns.begin(), fns.end());
    std::map<int, int> v2res;
    for (size_t i = 0; i < pv->size(); ++i) {
      auto v = (*pv)[i];
      std::cerr << "v = " << v << std::endl;
      for (auto& f : fns) {
        v = f(v);
        std::cerr << "v = " << v << std::endl;
      }
      std::cerr << "v = " << v << " @" << i << std::endl;
      if (v == target) {
        // fns_.clear();
        return i;
      }

    }
    // fns_.clear();
    return pv->size();
  }

 private:
  LazyArray* parent_ = nullptr;
  const std::vector<int> values_;
  MapFn fn_;
};

int main() {
  LazyArray a{{10, 20, 30, 40, 50}};
  std::function<int(int)> timesTwo = [](int i) { return i * 2; };
  assert(a.map(timesTwo).indexOf(40) == 1);
  assert(a.map(timesTwo).indexOf(40) == 1);
  assert(a.map(timesTwo).map(timesTwo).indexOf(80) == 1);
  auto b = a.map(timesTwo);
  auto c = b.map(timesTwo);
  assert(b.indexOf(80) == 3);

  assert(c.indexOf(80) == 1);

  assert(c.indexOf(80) == 1);

  assert(a.indexOf(50) == 4);

  assert(a.map(timesTwo).indexOf(101) == 5);

  int called = 0;
  std::function<int(int)> timesTwoForTest = [&](int i) { 
    ++called; return i * 2; };
  
  auto d = a.map(timesTwoForTest).map(timesTwoForTest);

  assert(called == 0);

  assert(d.indexOf(40) == 0);
  assert(2==called);
  {
    LazyArray a{{20, 10, 20, 30, 40, 50}};
    assert(a.map(timesTwo).map(timesTwo).indexOf(80) == 0);
  }
}



