
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

#include <latch>
#include <functional>

using namespace std;

class Foo {
public:
  Foo() : l1(1), l2(1) {}

  void first(function<void()> printFirst) {
    // printFirst() outputs "first". Do not change or remove this line.
    printFirst();
    l1.count_down();
  }

  void second(function<void()> printSecond) {
    l1.wait();
    // printSecond() outputs "second". Do not change or remove this line.
    printSecond();
    l2.count_down();
  }

  void third(function<void()> printThird) {
    l2.wait();
    // printThird() outputs "third". Do not change or remove this line.
    printThird();
  }
  std::latch l1, l2;
};

TEST_F(FooTest, Bar) {
  std::mutex mt;
  std::vector<int> output;
  auto getFn = [&](int v) {
    return [&, v] {
      std::lock_guard l(mt);
      LOG(INFO) << "Output " << v;
      output.push_back(v);
    };
  };

  Foo foo;
  std::thread t3([&]() { foo.third(getFn(3)); });
  std::thread t2([&]() { foo.second(getFn(2)); });
  std::thread t1([&]() { foo.first(getFn(1)); });

  t1.join();
  t2.join();
  t3.join();

  EXPECT_EQ(std::vector<int>({1, 2, 3}), output);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

