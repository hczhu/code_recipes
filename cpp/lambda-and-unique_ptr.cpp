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
#include <folly/Function.h>
// #include <gtest/gmock.h>

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

/* template end */

void foo(std::function<void()>&& f) {
  f();
}

TEST(Foo, Bar) {
  auto p = std::make_unique<int>(10);
  // Doesn't complile, because std::function requires the function object it
  // wraps to be CopyConstructible.
  // std::function<void()> f = [p = std::move(p)]() mutable { EXPECT_EQ(10, *p); };
  folly::Function<void()> f = [p = std::move(p)]() mutable {
    EXPECT_EQ(10, *p);
  };
  f();
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

