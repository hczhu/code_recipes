
#include <algorithm>
#include <type_traits>
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
// #include <gtest/gmock.h>

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

/* template end */

template<typename T>
struct is_bool_or_int : std::false_type {};

template<>
struct is_bool_or_int<bool> : std::true_type {};

template<>
struct is_bool_or_int<int> : std::true_type {};

template<typename T>
std::enable_if_t<is_bool_or_int<T>::value, bool> foo(T) {
  return true;
}

template<typename T>
std::enable_if_t<!is_bool_or_int<T>::value, bool> foo(T) {
  return false;
}

template<typename T, bool R = is_bool_or_int<T>::value>
bool bar(T) {
  return R;
}

TEST(Foo, Bar) {
  EXPECT_TRUE(foo(1));
  EXPECT_TRUE(foo(false));
  EXPECT_FALSE(foo(.33));
  EXPECT_FALSE(foo("adad"));

  EXPECT_TRUE(bar(1));
  EXPECT_TRUE(bar(false));
  EXPECT_FALSE(bar(.33));
  EXPECT_FALSE(bar("adad"));
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

