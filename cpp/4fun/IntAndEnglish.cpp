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
Mp name3 = {
  {1000000000, "billion"},
  {1000000, "million"},
  {1000, "thousand"},
};

Mp name2 = {
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

std::map<std::string, size_t> name3r, name2r;
const std::string hundred = "hundred";

std::string int2English(size_t val) {
  auto convert100 = [](size_t val) {
    std::string res;
    if (val / 100) {
      res += " " + name2[val / 100] + " " + hundred;
      val %= 100;
    }
    auto appendIf = [&](size_t val) {
      if (name2.count(val)) {
        res += " " + name2[val];
        return true;
      }
      return false;
    };
    if (!appendIf(val)) {
      appendIf(val / 10 * 10);
      appendIf(val % 10);
    }
    return res.substr(1);
  };
  std::string res;
  for (auto itr = name3.rbegin(); itr != name3.rend(); ++itr) {
    if (val / itr->first) {
      res += " " + convert100(val / itr->first) + " " + itr->second;
      val %= itr->first;
    }
  }
  if (val) {
    res += " " + convert100(val);
  }
  return res.substr(1);
}

size_t english2Int(const std::string& eng) {
  std::istringstream is(eng);
  std::string word;
  size_t res = 0;
  size_t below1000 = 0;
  while (is >> word) {
    if (name3r.count(word)) {
      res += below1000 * name3r[word];
      below1000 = 0;
    } else if (name2r.count(word)) {
      below1000 += name2r[word];
    } else if (word == hundred) {
      below1000 *= 100;
    } else {
      LOG(FATAL) << "Illegal input: " << eng;
    }
  }
  return res + below1000;
}

TEST(IntAndEnglish, Basic) {
  EXPECT_EQ("two billion one hundred three thousand fifteen", int2English(2000103015));
  EXPECT_EQ("eighty one million one hundred three thousand ten", int2English(81103010));
  EXPECT_EQ("one hundred thirty billion", int2English(130000000000));
  EXPECT_EQ("one hundred thirty billion fifty one million eight", int2English(130051000008));

  EXPECT_EQ(2000103015, english2Int("two billion one hundred three thousand fifteen"));
  EXPECT_EQ(81103010, english2Int("eighty one million one hundred three thousand ten"));
  EXPECT_EQ(130000000000, english2Int("one hundred thirty billion"));
  EXPECT_EQ(130051000008, english2Int("one hundred thirty billion fifty one million eight"));
}

TEST(IntAndEnglish, Everything) {
  size_t kMax = 2e9;
  for (size_t v = 1; v < kMax; v += (rand() % 10000)) {
    const auto english = int2English(v);
    // LOG_EVERY_N(INFO, 10000) << v << " = " << english;
    ASSERT_EQ(v, english2Int(english));
  }
  for (size_t v = 1; v <= kMax; v *= 10) {
    for (size_t d = 0; d < 100 && d < v; ++d) {
      ASSERT_EQ(v - d , english2Int(int2English(v - d)));
      ASSERT_EQ(v + d, english2Int(int2English(v + d)));
    }
  }
  for (size_t v = 1; v <= kMax; v = (v << 1) ^ (rand() & 1)) {
    ASSERT_EQ(v, english2Int(int2English(v)));
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  for (const auto& pr : name3) {
    name3r.emplace(pr.second, pr.first);
  }
  for (const auto& pr : name2) {
    name2r.emplace(pr.second, pr.first);
  }
  return RUN_ALL_TESTS();
}

