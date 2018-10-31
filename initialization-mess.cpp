
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
#include <gtest/gtest.h>

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

std::vector<int> echo(const std::vector<int>& v) {
  return v;
}

struct A {
  int a, b;
  A(int i, int j) : a(i), b(j) {}
};

/* template end */

TEST(InitializationMess, VectorMess) {
  EXPECT_EQ(2, echo({ 10, 20 }).size());
  EXPECT_EQ(std::vector<int>({10, 20}), echo({ 10, 20 }));
  // std::vector has initializer_list ctor.
  {
    std::vector<int> v{10};
    EXPECT_EQ(1, v.size());
  }
  {
    std::vector<int> v({10});
    EXPECT_EQ(1, v.size());
  }
  {
    std::vector<int> v = {10, 20, 30};
    EXPECT_EQ(3, v.size());
  }
  {
    std::vector<int> v(10, 20);
    EXPECT_EQ(10, v.size());
  }
  // std::string has initializer_list<char> ctor.
  {
    std::string s{10, 'a'};
    EXPECT_EQ(2, s.length());
    EXPECT_EQ('a', s[1]);
  }
  {
    std::string s({10, 'a'});
    EXPECT_EQ(2, s.length());
    EXPECT_EQ('a', s[1]);
  }
  {
    std::string s = {10, 'a'};
    EXPECT_EQ(2, s.length());
    EXPECT_EQ('a', s[1]);
  }
  {
    A a{1, 2};
    EXPECT_EQ(1, a.a);
    EXPECT_EQ(2, a.b);
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

