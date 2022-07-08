#include <utility>
#include <unordered_set>
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

class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};


struct Msg {
  int rec, sent, r;
  bool operator==(const Msg &rhs) const {
    return rec == rhs.rec && sent == rhs.sent && r == rhs.r;
  }
};

struct MsgHash {
  std::size_t operator()(const Msg& msg) const {
    std::size_t h1 = std::hash<decltype(msg.rec)>()(msg.rec);
    std::size_t h2 = std::hash<int>()(msg.sent);
    std::size_t h3 = std::hash<int>()(msg.r);

    return (h1 << 16) ^ (h2 << 8) ^ h3;
  }
};
using MsgSet = std::unordered_set<Msg, MsgHash>;

int binaryTreeReduce(std::vector<int>& values, MsgSet& rec, MsgSet& sent) {
  const auto n = values.size();
  for (size_t r = 1; r < n; r <<= 1) {
    LOG(INFO) << "---- Round #" << r << " ----------"; 
    const auto rr = r << 1;
    for (size_t i = 0; i + r < n; i += rr) {
      values[i] += values[i + r];
      LOG(INFO) << "Node #" << (i + r) << " sent a msg to node #" << i;
    }
  }
  return values[0];
}

TEST(Reduce, bianryTreeReduceTest) {
  auto testN = [](size_t n) {
    MsgSet recieved, sent;

    std::vector<int> values(n);
    for (size_t i = 0; i < n; ++i) {
      values[i] = i;
    }
    const auto ans = std::accumulate(values.begin(), values.end(), 0);

    EXPECT_EQ(binaryTreeReduce(values, recieved, sent), ans);

    EXPECT_EQ(recieved.size(), sent.size());
    for (const auto &pr : recieved) {
      EXPECT_EQ(1, sent.count(pr));
    }
  };

  SCOPED_TRACE("test 2"); testN(2);
  SCOPED_TRACE("test 9"); testN(9);
  SCOPED_TRACE("test 1"); testN(1);
  SCOPED_TRACE("test 101"); testN(101);


}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

