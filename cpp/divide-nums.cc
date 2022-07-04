
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

#define LSB(x) (((x) ^ ((x)-1)) & (x))

std::vector<std::vector<int>> getInvertedIndex(int n, const int nums[]) {
  std::vector<int> bcount(1 << n);
  std::vector<int> sum(1 << n);
  std::vector<std::vector<int>> n2sum(n + 1);
  n2sum[0].push_back(0);

  for (int i = 0; i < n; ++i) {
    sum[1 << i] = nums[i];
  }
  for (int b = 1; b < (1 << n); ++b) {
    sum[b] = sum[LSB(b)] + sum[b ^ LSB(b)];
    bcount[b] = bcount[b ^ LSB(b)] + 1;
    n2sum[bcount[b]].push_back(sum[b]);
  }

  for (auto& sums : n2sum) {
    std::sort(sums.begin(), sums.end());
    auto sz = std::unique(sums.begin(), sums.end()) - sums.begin();
    sums.resize(sz);
  }
  return n2sum;
}

int minimumDifference(std::vector<int> &nums) {

  int allSum = 0;
  for (auto num : nums) {
    allSum += num;
  }
  const size_t n = nums.size() >> 1;

  const auto n2sum1 = getInvertedIndex(n, &nums[0]);
  const auto n2sum2 = getInvertedIndex(n, &nums[n]);

  int res = 1e9;
  for (int a = 0; a < n; ++a) {
    auto& sum1 = n2sum1[a];
    auto sum2 = n2sum2[n - a];
    int p2 = sum2.size() - 1;
    for (int s1 : sum1) {
      while (p2 >=0 && 2 * (s1 + sum2[p2]) > allSum) {
        res = std::min(res, std::abs(allSum - 2 * (s1 + sum2[p2])));
        --p2;
      }
      if (p2 >= 0) {
        res = std::min(res, std::abs(allSum - 2 * (s1 + sum2[p2])));
        if (res == 0) {
          return 0;
        }
      }
    }
  }

  return res;
}

TEST_F(FooTest, Bar) {

  {
    std::vector<int> nums = {
      36, -36,
    };
    EXPECT_EQ(minimumDifference(nums), 72);
  }
  {
    std::vector<int> nums = {
      1, 2, 3, 4
    };
    EXPECT_EQ(minimumDifference(nums), 0);
  }
  {
    std::vector<int> nums = {
      2,-1,0,4,-2,-9
    };
    EXPECT_EQ(minimumDifference(nums), 0);
  }
  {
    std::vector<int> nums(30);
    for (int i = 0; i < 30; ++i) {
      nums[i] = i - 15;
    }
    EXPECT_EQ(minimumDifference(nums), 1);
  }
  {
    std::vector<int> nums = {-5821460, 7291170,  5435514,  -7338773, 683339,
                             339817,   7179739,  -7334749, 2479133,  8371812,
                             4498960,  -5751228, -191901,  1479381,  -4639233,
                             1788977,  1741855,  9700706,  1551616,  9917210,
                             7456677,  2056436,  847606,   8699791,  -6659757,
                             127681,   0,        1397910,  -4337523, -6592872};
    EXPECT_EQ(minimumDifference(nums), 0);
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

