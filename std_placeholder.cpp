
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


template<class OutputType>
std::enable_if_t<std::is_placeholder<OutputType>::value == 0, void>
toMaybePlaceholder(std::string input, OutputType& output) {
  PEEK(input);
  output = input;
}

template<class OutputType>
std::enable_if_t<std::is_placeholder<OutputType>::value, void>
  toMaybePlaceholder(std::string, OutputType& output) {
}

TEST(Foo, Bar) {
  std::string output;
  toMaybePlaceholder("haha", output);
  EXPECT_EQ("haha", output);
  
  toMaybePlaceholder("", std::placeholders::_1);
  EXPECT_EQ("haha", output);
  toMaybePlaceholder("", std::placeholders::_2);
  EXPECT_EQ("haha", output);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

