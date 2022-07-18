
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
#include <unordered_map>
#include <list>

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

class LfuCatchTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

template <typename K, typename V>
class LfuCache {
 private:
  struct Node {
    V v;
    // 'itr' refers to a node in 'freq2Keys_'
    const std::list<K>::const_iterator itr;
    // How many times has this item been accessed (reads/writes)?
    size_t f = 1;
  };

 public:
  LfuCache(size_t capacity) : capacity_(capacity) {
    CHECK_GT(capacity_, 0);
  }

  void put(const K& k, const V& v) {
    if (auto vPtr = get(k)) {
      *vPtr = v;
      return;
    }

    if (key2Node_.size() == capacity_) {
      auto& lowestKeyList = freq2Keys_[lowestFreq_];
      CHECK(!lowestKeyList.empty()) << lowestFreq_;
      const auto& keyToEvict = *lowestKeyList.rbegin();
      LOG(INFO) << "Evicting " << keyToEvict;
      key2Node_.erase(keyToEvict);
      lowestKeyList.pop_back();
      if (lowestKeyList.empty()) {
        // lowestFreq_ is invalid now and has to be reset later
        freq2Keys_.erase(lowestFreq_);
      }
    }

    lowestFreq_ = 1;
    auto& lowestKeyList = freq2Keys_[lowestFreq_];
    lowestKeyList.insert(lowestKeyList.begin(), k);
    Node newNode{
      .v = v,
      .itr = lowestKeyList.begin(),
    };
    key2Node_.emplace(k, newNode);
    LOG(INFO) << "Put " << k << " = " << v;
  }

  void incrementAccess(Node& node) {
    auto& curKeyList = freq2Keys_[node.f];
    auto& nextKeyList = freq2Keys_[++node.f];

    nextKeyList.splice(nextKeyList.begin(), curKeyList, node.itr);

    if (curKeyList.empty()) {
      freq2Keys_.erase(node.f - 1);
      if (lowestFreq_ + 1 == node.f) {
        ++lowestFreq_;
      }
    }
  }

  V* get(const K& k) {
    const auto itr = key2Node_.find(k);
    if (itr == key2Node_.end()) {
      return nullptr;
    }
    auto& node = itr->second;
    incrementAccess(node);
    LOG(INFO) << "get " << k;
    return &node.v;
  }

 private:
  const size_t capacity_;
  // Frequency to key list sorted by recency 
  std::unordered_map<size_t, std::list<K>> freq2Keys_;

  std::unordered_map<K, Node> key2Node_;

  size_t lowestFreq_ = 1;

};

TEST_F(LfuCatchTest, Basic) {
  LfuCache<int, int> lfu(2);

  auto get = [&lfu](int k) {
    auto vPtr = lfu.get(k);
    return vPtr == nullptr ? -1 : *vPtr;
  };

  lfu.put(1, 1); // cache=[1,_], cnt(1)=1
  lfu.put(2, 2); // cache=[2,1], cnt(2)=1, cnt(1)=1
  EXPECT_EQ(1, get(1));    // return 1
                 // cache=[1,2], cnt(2)=1, cnt(1)=2
  lfu.put(3, 3); // 2 is the LFU key because cnt(2)=1 is the smallest,
                 // invalidate 2. cache=[3,1], cnt(3)=1, cnt(1)=2
  EXPECT_EQ(-1, get(2));    // return -1 (not found)
  EXPECT_EQ(3, get(3));    // return 3
                 // cache=[3,1], cnt(3)=2, cnt(1)=2
  lfu.put(4, 4); // Both 1 and 3 have the same cnt, but 1 is LRU, invalidate 1.
                 // cache=[4,3], cnt(4)=1, cnt(3)=2
  EXPECT_EQ(-1, get(1));    // return -1 (not found)

  EXPECT_EQ(3, get(3));    // return 3
                 // cache=[3,4], cnt(4)=1, cnt(3)=3
  EXPECT_EQ(4, get(4));    // return 4
                 // cache=[4,3], cnt(4)=2, cnt(3)=3
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

