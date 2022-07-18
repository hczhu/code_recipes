
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

// The top of the heap is the smallest element.
// Can't have duplicate keys.
// The memory footprint is linear with the maximum value of keys.
template <typename Cmp = std::less<size_t>> class BinaryHeap {
public:
  BinaryHeap(size_t cap, Cmp &&cmpLess)
      : heap_(cap), hpos_(cap), hs_(0), cmpLess_(std::move(cmpLess)) {}

  BinaryHeap(Cmp &&cmpLess, const std::vector<size_t> &keys)
      : BinaryHeap(keys.size(), std::move(cmpLess)) {
        // TODO: implement heapifying an array
  }

  void adjustUp(size_t key) & {
    auto pos = hpos_[key];
    LOG(INFO) << "----- Adjusting key up " << key << " @" << pos << " ----";
    CHECK_LT(pos, hs_);
    CHECK_EQ(heap_[pos], key);
    while (pos && cmpLess_(key, heap_[(pos - 1) >> 1])) {
      LOG(INFO) << "Pos: " << pos << " for key " << key << " is moving up.";
      heap_[pos] = heap_[(pos - 1) >> 1];
      hpos_[heap_[pos]] = pos;
      pos = (pos - 1) >> 1;
    }
    heap_[pos] = key;
    hpos_[key] = pos;
  }

  void adjustDown(size_t key) {
    auto pos = hpos_[key];
    CHECK_LT(pos, hs_);
    CHECK_EQ(heap_[pos], key);

    while ((pos << 1) + 1 < hs_) {
      auto mm = 1 + (pos << 1);
      if (mm + 1 < hs_ && cmpLess_(heap_[mm + 1], heap_[mm])) {
        mm++;
      }
      if (cmpLess_(key, heap_[mm])) {
        break;
      }
      heap_[pos] = heap_[mm];
      hpos_[heap_[pos]] = pos;
      pos = mm;
    }
    heap_[pos] = key;
    hpos_[key] = pos;
  }

  size_t pop() {
    auto re = heap_[0];
    hs_--;
    if (hs_) {
      heap_[0] = heap_[hs_];
      hpos_[heap_[0]] = 0;
      adjustDown(heap_[0]);
    }
    return re;
  }

  void erase(size_t key) {
    auto pos = hpos_[key];
    heap_[pos] = heap_[hs_ - 1];
    hpos_[heap_[pos]] = pos;
    hs_--;
    if (hs_) {
      adjustUp(heap_[pos]);
      adjustDown(heap_[pos]);
    }
  }

  void push(size_t key) {
    if (hs_ >= heap_.size() || key >= hs_) {
      heap_.resize(std::max(hs_ + 1, key + 1));
      hpos_.resize(heap_.size());
    }
    heap_[hs_] = key;
    hpos_[key] = hs_;
    ++hs_;
    adjustUp(key);
  }

  size_t size() const { return hs_; };
  void clear() & { hs_ = 0; };
  size_t top() const { return heap_[0]; }

private:
  std::vector<size_t> heap_;
  // 'hpos_[k]' gives the index in 'heap_' for key 'k'.
  std::vector<size_t> hpos_;
  // The size of the heap.
  size_t hs_;
  Cmp cmpLess_;
};

struct Cmp {
  Cmp(const std::vector<int> &values) : v(values) {}

  bool operator()(size_t a, size_t b) const {
    LOG(INFO) << "Cmping " << a << " with " << b;
    return std::tie(v[a], a) > std::tie(v[b], b);
  }

  const std::vector<int> &v;
};

TEST(HeapTest, Basic) {
  std::vector<int> values = {
    0, 1 ,2, 3, 4, 5,6
  };
  BinaryHeap<Cmp> heap(4, Cmp(values));
  heap.push(1);
  heap.push(0);

  EXPECT_EQ(heap.top(), 1);

  heap.push(4);
  heap.push(6);

  EXPECT_EQ(heap.top(), 6);
  EXPECT_EQ(heap.pop(), 6);
  EXPECT_EQ(heap.top(), 4);

  values[0] = 7;

  heap.adjustUp(0);
  EXPECT_EQ(heap.top(), 0);

  EXPECT_EQ(3, heap.size());

  heap.push(6);
  EXPECT_EQ(heap.top(), 0);
  values[0] = 5;
  heap.adjustDown(0);
  EXPECT_EQ(heap.top(), 6);

  heap.erase(0);
  EXPECT_EQ(3, heap.size());
  EXPECT_EQ(heap.pop(), 6);
  EXPECT_EQ(heap.pop(), 4);
  EXPECT_EQ(heap.pop(), 1);
  
  EXPECT_EQ(0, heap.size());

}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

