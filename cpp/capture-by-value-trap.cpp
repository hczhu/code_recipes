
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

class CaptureByValueTrap : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(CaptureByValueTrap, Basic) {
  int v = 0;
  auto f = [=] ()mutable {
    return ++v;      
  };

  EXPECT_EQ(1, f());
  EXPECT_EQ(2, f());
  EXPECT_EQ(3, f());

  EXPECT_EQ(0, v);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

