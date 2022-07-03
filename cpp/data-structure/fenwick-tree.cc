
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

size_t ceilingTwo(size_t n) {
  size_t res = 1;
  while (res < n) {
    res <<= 1;
  }
  return res;
}

// Least significant bit
#define LSB(x) (((x) ^ ((x) - 1)) & (x))

// Support cumulative operations, e.g. summation, on segments.
template<typename Segment>
class FenwickTree {
 public:
   using SegCallback = std::function<void(Segment &)>;

   FenwickTree(size_t n) : n_(ceilingTwo(n)), points_(n_ + 1), segs_(n_ + 1) {}

   size_t size() const { return n_; }

   Segment &getPoint(size_t idx) & { return points_[idx]; }

   // Visit all parent segments consisting of 'pointIdx' from the shortest one
   // to the longest one.
   // E.g. 18 (10010) will visit the following segments in the order
   //    18 (10010) => (16, 18]
   //    20 (10100) => (16, 20]
   //    24 (11000) => (16, 24]
   //    32 (100000) => (0, 32]
   //    assuming n_ is 32
   void traverseParentSegments(size_t pointIdx, SegCallback &&fn) {
     if (pointIdx == 0) {
       return;
     }
     for (size_t segIdx = pointIdx; segIdx <= n_; segIdx += LSB(segIdx)) {
       fn(segs_[segIdx]);
     }
   }

   // Visit all a disjoint set of segments which combined exactly cover [l, r)
   // from the rightmost one to the leftmost one. E.g. [3, 8) or (2, 7] will be
   // covered by segments and points
   //     (6, 7]
   //     (4, 6]
   //     4  (point)
   //     (2, 3]
   //
   // 'fn' can record cumulative operations on a segment covered by [l, r).
   void traverseSegmentsFor(size_t l, size_t r, SegCallback &&fn) {
     --l;
     --r;
     // To cover (l, r] now.
     while (r > l) {
       auto nr = r - LSB(r);
       if (nr >= l) {
         fn(segs_[r]);
         r = nr;
       } else {
         fn(points_[r]);
         --r;
       }
     }
   }

   void traverseChildSegments(size_t segIdx, SegCallback &&fn) {
     fn(points_[segIdx]);
     size_t l = segIdx - LSB(segIdx) + 1;
     size_t r = segIdx;
     traverseSegmentsFor(l, r, std::move(fn));
   }

   size_t grow() {
     n_ <<= 1;
     points_.resize(n_ + 1);
     segs_.resize(n_ + 1);
     return n_;
   }

 protected: 
  // Support all indexes in [1, n_].
  size_t n_;
  // 1-based index. points_[0] is not used.
  std::vector<Segment> points_;
  // 1-based index. segs_[0] is not used.
  // segs_[i] represents segment (i - LSB(i), i].
  // E.g. segs_[8] => (0, 8]
  //      segs_[7] => (6, 7]
  //      segs_[6] => (4, 6]
  std::vector<Segment> segs_;
};

class SegmentTreeTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SegmentTreeTest, summation) {
  FenwickTree<int> sumTree(3);
  // A segment [l ,r) records the sum of all ops covering the whole segment.  
  sumTree.getPoint(3) += 3;

  sumTree.traverseSegmentsFor(1, 4, [](int &sum) { sum += 1; });

  sumTree.grow();
  auto expect = [&](size_t pidx) {
    int res = sumTree.getPoint(pidx);
    sumTree.traverseParentSegments(pidx,
                                   [&res](int &sum) { res += sum; });
    return res;
  };
  EXPECT_EQ(1, expect(1));
  EXPECT_EQ(1, expect(2));
  EXPECT_EQ(4, expect(3));

  sumTree.traverseSegmentsFor(3, 7, [](int &sum) { sum -= 2; });

  EXPECT_EQ(1, expect(2));
  EXPECT_EQ(2, expect(3));
  EXPECT_EQ(-2, expect(4));
  EXPECT_EQ(-2, expect(5));
  EXPECT_EQ(-2, expect(6));
  EXPECT_EQ(0, expect(7));
  EXPECT_EQ(0, expect(8));
}

struct Seg {
  // a * x + b
  size_t x = 0;
  size_t a = 1;
  size_t b = 0;
};

class Fancy {
  static constexpr size_t M = 1e9 + 7;

public:
  Fancy() : tree_(2) {}

  void append(int val) {
    ++n_;
    while (n_ > tree_.size()) {
      tree_.grow();
    }
    tree_.getPoint(n_).x = size_t(val);
  }

  void addAll(int inc) {
    tree_.traverseSegmentsFor(1, n_ + 1,
                              [inc](Seg &seg) { seg.b = (seg.b + inc) % M; });
  }

  void multAll(int m) {
    tree_.traverseSegmentsFor(1, n_ + 1, [m](Seg &seg) {
      seg.a = (seg.a * m) % M;
      seg.b = (seg.b * m) % M;
    });
  }

  int getIndex(int idx) {
    ++idx;
    if (idx > n_) {
      return -1;
    }
    auto val = tree_.getPoint(idx);
    size_t res = ((val.a * val.x) + val.b) % M;
    std::cout << val.a << " * " << val.x << " + " << val.b << std::endl;
    tree_.traverseParentSegments(idx, [&res](Seg &seg) {
      std::cout << seg.a << " * " << res << " + " << seg.b << std::endl;
      res = ((res * seg.a) + seg.b) % M;
    });

    std::cout << "==========" << std::endl;
    return res;
  }

private:
  FenwickTree<Seg> tree_;
  size_t n_ = 0;
};

TEST_F(SegmentTreeTest, LeetCodeFancy) {
  Fancy fancy;

  fancy.append(2);   // fancy sequence: [2]
  fancy.addAll(3);   // fancy sequence: [2+3] -> [5]
  EXPECT_EQ(5, fancy.getIndex(0)); // return 10
  fancy.append(7);   // fancy sequence: [5, 7]
  fancy.multAll(2);  // fancy sequence: [5*2, 7*2] -> [10, 14]
  EXPECT_EQ(10, fancy.getIndex(0)); // return 10

  fancy.addAll(3);   // fancy sequence: [10+3, 14+3] -> [13, 17]
  fancy.append(10);  // fancy sequence: [13, 17, 10]
  fancy.multAll(2);  // fancy sequence: [13*2, 17*2, 10*2] -> [26, 34, 20]

  EXPECT_EQ(26, fancy.getIndex(0)); // return 26
  EXPECT_EQ(34, fancy.getIndex(1)); // return 34
  EXPECT_EQ(20, fancy.getIndex(2)); // return 20
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

