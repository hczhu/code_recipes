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
#include <unordered_set>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

/* template end */


template<typename K, typename V>
class LruCache {
 public:
  explicit LruCache(size_t maxItems) : maxItems_(maxItems) {
    head_.next = &tail_;
    tail_.prev = &head_;
  }

  void set(const K &k, const V &v) {
    if (auto ptr = getImp(k)) {
      ptr->v = v;
      return;
    }

    if (nodes_.size() == maxItems_) {
      const auto deleted = nodes_.erase(*popTail());
      CHECK_EQ(1, deleted);
    }

    const auto pr = nodes_.insert(Node{
        .k = k,
        .v = v,
    });
    CHECK(pr.second);
    pushToHead(&*(pr.first));
  }

  const V* get(const K& k) {
    auto ptr = getImp(k);
    return ptr ? &ptr->v : nullptr;
  }

 private:
  struct Node {
    K k;
    mutable V v;
    mutable const Node* prev = nullptr;
    mutable const Node* next = nullptr; 
  };
  struct Hasher {
    size_t operator()(const Node &node) const noexcept {
      return hasher(node.k);
    }

    std::hash<K> hasher;
  };
  struct Equal {
    bool operator()(const Node &l, const Node &r) const { return l.k == r.k; }
  };

  using Set = std::unordered_set<Node, Hasher, Equal>;

  const Node* getImp(const K& k) {
    Node node{
      .k = k,
    };
    auto itr = nodes_.find(node);
    if (itr == nodes_.end()) {
      return nullptr;
    }
    auto nodePtr = &(*itr);
    unlinkNode(nodePtr);
    pushToHead(nodePtr);
    return nodePtr;
  }
  void unlinkNode(const Node* nodePtr) {
    nodePtr->prev->next = nodePtr->next;
    nodePtr->next->prev = nodePtr->prev;
  }

  void pushToHead(const Node* nodePtr) {
    nodePtr->next = head_.next;
    nodePtr->next->prev = nodePtr;
    nodePtr->prev = &head_;
    head_.next = nodePtr;
  }

  const Node* popTail() {
    auto nodePtr = tail_.prev;
    nodePtr->prev->next = &tail_;  
    tail_.prev = nodePtr->prev;
    return nodePtr;
  }

  const size_t maxItems_;
  Node head_;
  Node tail_;
  Set nodes_;
};

class LruCacheTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(LruCacheTest, Simple) {
  LruCache<int, int> cache(3);
  EXPECT_EQ(nullptr, cache.get(0));
  cache.set(0, 0);
  EXPECT_EQ(0, *cache.get(0));
  cache.set(0, 1);
  EXPECT_EQ(1, *cache.get(0));

  cache.set(1, 1);
  cache.set(2, 2);
  EXPECT_EQ(1, *cache.get(0));
  EXPECT_EQ(2, *cache.get(2));
  EXPECT_EQ(1, *cache.get(1));

  cache.set(3, 3);
  EXPECT_EQ(nullptr, cache.get(0));

  cache.set(4, 4);
  EXPECT_EQ(nullptr, cache.get(2));

}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

