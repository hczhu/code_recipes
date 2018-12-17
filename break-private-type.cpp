
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

class Foo {
  struct PrivateTag {};

 public:
  Foo(int a, PrivateTag) : a_(a) {}

  void bar(PrivateTag) {}

  static void sbar(PrivateTag) {}

 private:
  const int a_;
};

template<typename T>
T extract(void(Foo::*)(T)) {
  return T();
}

template<typename T>
T extractByRef(void(&funRef)(T)) {
  funRef(T());
  return T();
}


TEST(Foo, Bar) {
  // Foo foo0(-1, extract(&Foo::Foo));

  Foo foo(-1, extract(&Foo::bar));
  Foo foo1(-1, extractByRef(Foo::sbar));
  auto p = &Foo::bar;
  // Doesn't compilej
  // auto& pref = &Foo::bar;
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

