
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

#include "dummy-classes.h"

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

// #define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

/* template end */

class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(FooTest, Bar) {
  Base a;
  {
    Base d;
    static Base c;
  }
  EXPECT_EQ(2, Base::LiveCnt);
  for (bool once = false; once;) {
    for (static Base d; once; once = false) {
    }
  }
  EXPECT_EQ(2, Base::LiveCnt);
  auto logIt = [] {
    for (bool once = true; once;) {
      for (static Base d; once; once = false) {
      }
    }
  };
  std::vector<std::thread> threads;
  for (int t = 0; t < 10; ++t) {
    threads.emplace_back([logIt] {
      for (int i = 0; i < 100; ++i) {
        logIt();
        EXPECT_EQ(3, Base::LiveCnt);
        EXPECT_EQ(1, Base::DtorCnt);
      }
    });
  }
  for (auto& thr : threads) {
    thr.join();
  }
  EXPECT_EQ(3, Base::LiveCnt);
  EXPECT_EQ(1, Base::DtorCnt);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

