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

using Mp = std::map<size_t, std::string>;
const Mp name3 = {
  {1000000000, "billion"},
  {1000000, "million"},
  {1000, "thousand"},
};

const Mp name2 = {
  {1, "one"},
  {2, "two"},
  {3, "three"},
  {4, "four"},
  {5, "five"},
  {6, "six"},
  {7, "seven"},
  {8, "eight"},
  {9, "nine"},

  {10, "ten"},
  {11, "eleven"},
  {12, "twelve"},
  {13, "thirteen"},
  {14, "fourteen"},
  {15, "fifteen"},
  {16, "sixteen"},
  {17, "seventeen"},
  {18, "eighteen"},
  {19, "nineteen"},

  {20, "twenty"},
  {30, "thirty"},
  {40, "fourty"},
  {50, "fifty"},
  {60, "sixty"},
  {70, "seventy"},
  {80, "eighty"},
  {90, "ninty"},
};

std::string int2English(size_t val) {
  auto convert100 = [](size_t val) {
    return
      val / 100 ? ()
  };
}

size_t english2Int(const std::string& eng) {

}

TEST(IntAndEnglish, huge) {
  for (size_t v = 1; v < 1e10; ++v) {
    EXPECT_EQ(v, englis2Int(int2English(v)));
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

