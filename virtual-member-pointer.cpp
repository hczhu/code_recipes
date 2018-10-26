
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

/* template end */

class A {
 public:
  virtual ~A() = default;
  virtual int foo() {
    return 0;
  }
};

class B : public A {
 public:
  int foo() override {
    return 1;
  }

  std::vector<int> bar(int, double) {
    return {
      0
    };
  }
  std::vector<int> bar(double) {
    return {
      1
    };
  }
  template <typename R, typename... Args>
  using method_ptr_t = std::vector<R> (B::*)(int, Args...);

  template <typename R, typename... Args>
  std::vector<R> forward(method_ptr_t<R, Args...> m, int v, Args&&... args);
};

template <typename R, typename... Args>
std::vector<R> B::forward(method_ptr_t<R, Args...> m, int v, Args&&... args) {
  return (this->*m)(v, std::forward<Args>(args)...);
}

TEST(Foo, Bar) {
  B b;
  A* a = &b;
  using Mptr = int(A::*)();
  Mptr ptr = &A::foo;
  LOG(INFO) << "virtual method pointer: " << ptr;
  EXPECT_EQ(1, (a->*ptr)());

  EXPECT_EQ(std::vector<int>({0}), b.forward(&B::bar, 0, 1.1));
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

