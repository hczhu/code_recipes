
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

class SerializedStringSet {
 public:
  using Len = uint32_t;
  explicit SerializedStringSet(std::vector<std::string> strings) {
    std::sort(strings.begin(), strings.end());
    // the nubmer of strings + the offsets of each string + sentinel 
    auto total_len = sizeof(Len) * (2 + strings.size());
    for (const auto& str : strings) {
      total_len += str.length();
    }
    // TODO: align to uint32_t
    buf_ = std::make_unique<char[]>(total_len);
    LOG(INFO) << "Allocated " << total_len << " bytes.";
    auto buf = buf_.get();
    auto writeOneUint = [&](Len v) {
      *reinterpret_cast<Len*>(buf) = v;
      buf += sizeof(Len);
    };
    writeOneUint(strings.size());
    Len offsets = 0;
    for (const auto& str : strings) {
      writeOneUint(offsets);
      offsets += str.length();
    }
    writeOneUint(offsets);
    for (const auto& str : strings) {
      std::copy(str.begin(), str.end(), buf);
      buf += str.length();
    }
    CHECK_EQ(buf - buf_.get(), total_len);
  }

  bool hasStr(const std::string& str) const {
    auto buf = buf_.get();
    auto readOne = [&]() {
      auto ret = *reinterpret_cast<Len*>(buf);
      buf += sizeof(Len);
      return ret;
    };
    const Len N = readOne();
    auto compTarget = [&](Len start, Len end) {
      for (int i = 0; i < str.size() && i < (end - start); ++i) {
        if (str[i] != buf[start + i]) {
          return (buf[start + i] < str[i]) ? -1 : 1;
        }
      }
      // One is a prefix of the other.
      if ((end -start) != str.size()) {
        return (end - start) < str.size() ? -1 : 1;
      }
      return 0;
    };
    Len low = 0, high = N;
    const Len* offsets = reinterpret_cast<Len*>(buf);
    buf += (N + 1) * sizeof(Len);
    while (low < high) {
      Len mid = (low + high) / 2;
      if (compTarget(offsets[mid], offsets[mid + 1]) < 0) {
        low = mid + 1;
      } else {
        // invariant: offsets[high] >= target
        high = mid;
      }
    }
    return high < N && compTarget(offsets[high], offsets[high + 1]) == 0;
  }

 private:
  std::unique_ptr<char[]> buf_;
};

TEST(SimpleTest, Simple) {
  SerializedStringSet sss(std::vector<std::string>({
      "bb",
      "b",
      "aaa",
      "aa",
      "a",
      "bb",
      "",
      "c",
  }));
  EXPECT_TRUE(sss.hasStr("a"));
  EXPECT_TRUE(sss.hasStr("aa"));
  EXPECT_TRUE(sss.hasStr("aaa"));
  EXPECT_FALSE(sss.hasStr("aaaa"));

  EXPECT_FALSE(sss.hasStr("d"));
  EXPECT_FALSE(sss.hasStr("aad"));

  EXPECT_TRUE(sss.hasStr(""));
  EXPECT_TRUE(sss.hasStr("bb"));
  EXPECT_FALSE(sss.hasStr("bbb"));

  EXPECT_FALSE(sss.hasStr("cc"));
}

TEST(SimpleTest, CornerCase) {
  SerializedStringSet sss(std::vector<std::string>({}));
  EXPECT_FALSE(sss.hasStr("aa"));
  EXPECT_FALSE(sss.hasStr(""));
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

