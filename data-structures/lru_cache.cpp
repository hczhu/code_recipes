#include "../header.h"


template <typename K, typename V>
class LruCache {
 public:
  explicit LruCache(size_t maxItems) : maxItems_(maxItems) {}
  virtual ~LruCache() = default;

  virtual void set(const K& k, const V& v) = 0;
  virtual const V* get(const K& k) = 0;

 protected:
  const size_t maxItems_;
};

template <typename K,
          typename V,
          template <typename, typename, typename>
          typename HashSet>
class LruCacheWithPointers : public LruCache<K, V> {
 public:
  using Super = LruCache<K, V>;

  explicit LruCacheWithPointers(size_t maxItems) : LruCache<K, V>(maxItems) {
    head_.next = &tail_;
    tail_.prev = &head_;
  }

  void set(const K& k, const V& v) override {
    if (auto ptr = getImp(k)) {
      ptr->v = v;
      return;
    }

    if (nodes_.size() == Super::maxItems_) {
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

  const V* get(const K& k) override {
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
    size_t operator()(const Node& node) const noexcept {
      return hasher(node.k);
    }

    std::hash<K> hasher;
  };
  struct Equal {
    bool operator()(const Node& l, const Node& r) const { return l.k == r.k; }
  };

  using Set = HashSet<Node, Hasher, Equal>;

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

  Node head_;
  Node tail_;
  Set nodes_;
};

template <typename K,
          typename V,
          template <typename, typename, typename, typename>
          typename HashSet>
class LruCacheWithIndexes : public LruCache<K, V> {
 public:
  using Super = LruCache<K, V>;
  using Idx = uint32_t;

  explicit LruCacheWithIndexes(size_t maxItems)
      : LruCache<K, V>(maxItems),
        nodes_(maxItems + 2),
        tail_(maxItems + 1),
        nodeSet_(maxItems, Allocator(nodes_)) {
    nodes_[0].next = 1;
    nodes_[tail_].prev = tail_ - 1;
    for (Idx i = 1; i <= Super::maxItems_; ++i) {
      nodes_[i].prev = i - 1;
      nodes_[i].next = i + 1;
    }
  }

  void set(const K& k, const V& v) override {
    if (auto ptr = getImp(k)) {
      ptr->v = v;
      return;
    }

    if (nodes_.size() == Super::maxItems_) {
      const auto deleted = nodeSet_.erase(nodes_[nodes_[0].next]);
      CHECK_EQ(1, deleted);
    }

    Node node{
        .k = k,
        .v = v,
    };
    const auto pr = nodeSet_.insert(std::move(node));
    CHECK(pr.second);

    const Idx idx = nodes_[0].next;
    CHECK_EQ(nodes_[idx].k, k);
    CHECK_EQ(nodes_[idx].v, v);

    nodes_[0].next = nodes_[idx].next;
    nodes_[nodes_[0].next].prev = 0;

    nodes_[idx].next = tail_;
    nodes_[idx].prev = nodes_[tail_].prev;
    nodes_[tail_].prev = idx;
    nodes_[nodes_[idx].prev].next = idx;
  }

  const V* get(const K& k) override {
    auto ptr = getImp(k);
    return ptr ? &ptr->v : nullptr;
  }

 private:
  struct Node {
    K k;
    mutable V v;
    Idx prev;
    Idx next;
  };
  std::vector<Node> nodes_;
  const Idx tail_;

  struct Hasher {
    size_t operator()(const Node& node) const noexcept {
      return hasher(node.k);
    }
    std::hash<K> hasher;
  };
  struct Equal {
    bool operator()(const Node& l, const Node& r) const { return l.k == r.k; }
  };
  struct Allocator : std::allocator<Node> {
    explicit Allocator(std::vector<Node>& nodesArg) : nodes(nodesArg) {
      LOG(INFO) << "Constructed an Allocator @" << this;
    }

    Node* allocate(std::size_t n) {
      CHECK_EQ(1, n) << "Can only allocate one node.";
      Idx ret = nodes[0].next;
      nodes[0].next = nodes[ret].next;
      nodes[nodes[0].next].prev = 0;
      LOG(INFO) << "Allocated node @" << ret;
      return &nodes[ret];
    }

    void deallocate(Node* p, std::size_t n) {
      CHECK_EQ(1, n) << "Can only deallocate one node.";
      const auto idx = p - &nodes[0];
      nodes[idx].next = nodes[0].next;
      nodes[idx].prev = 0;
      nodes[nodes[idx].next].prev = idx;
      nodes[0].next = idx;
      LOG(INFO) << "Deallocated node @" << idx;
    }

    template <typename U>
    using other = std::allocator<U>;

    std::vector<Node>& nodes;
  };

  using Set = HashSet<Node, Hasher, Equal, Allocator>;
  Set nodeSet_;

  const Node* getImp(const K& k) {
    Node node{
        .k = k,
    };
    auto itr = nodeSet_.find(node);
    if (itr == nodeSet_.end()) {
      return nullptr;
    }
    const auto idx = &(*itr) - &nodes_[0];

    nodes_[nodes_[idx].prev].next = nodes_[idx].next;
    nodes_[nodes_[idx].next].prev = nodes_[idx].prev;

    nodes_[idx].next = tail_;
    nodes_[idx].prev = nodes_[tail_].prev;
    nodes_[nodes_[idx].prev].next = idx;
    nodes_[nodes_[idx].next].prev = idx;
    return &nodes_[idx];
  }
};

class LruCacheTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}

  template <template <typename, typename> typename LruCache>
  void doTest() {
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
};

template <typename K, typename V>
using LruCacheWithPointersAndStdUnorderedSet =
    LruCacheWithPointers<K, V, std::unordered_set>;

template <typename K, typename V>
using LruCacheWithIndexesAndStdUnorderedSet =
    LruCacheWithIndexes<K, V, std::unordered_set>;


#include <list>

template <typename K, typename V>
class LruCacheList {
 public:
  using Super = LruCache<K, V>;
  
  LruCacheList(size_t maxNumItems) : maxNumItems_(maxNumItems) {
  }

  void printList(const std::string& header) const {
    std::cout << header;
    for (auto itr = recencyList_.begin(); itr != recencyList_.end(); ++itr) {
      std::cout << " " << *itr;
    }
    std::cout << std::endl;
  }

  void set(const K& k, const V& v) {
    if (auto vPtr = get(k)) {
      *vPtr = v;
      return;
    }
    if (k2Node_.size() == maxNumItems_) {
      const auto& keyToEvict = *recencyList_.rbegin();
      LOG(INFO) << "Evicting " << keyToEvict;
      const auto numErased = k2Node_.erase(keyToEvict);
      CHECK_EQ(1, numErased);
      recencyList_.pop_back();
    }

    recencyList_.insert(recencyList_.begin(), k);
    Node node{
      .v = v,
      .itr = recencyList_.begin(),
    };
    k2Node_.emplace(k, node);
    printList("set()");
  }
  
  V* get(const K& k) {
    auto itr = k2Node_.find(k);
    if (itr == k2Node_.end()) {
      return nullptr;
    }
    auto& node = itr->second;
    recencyList_.splice(recencyList_.begin(), recencyList_, node.itr);
    printList("get()");
    return &node.v;
  }

 private:
  const size_t maxNumItems_;
  // .begin() is the most recently accessed key.
  std::list<K> recencyList_;

  struct Node {
    V v;
    // refer to the element in the 'recencyList_'
    const typename decltype(recencyList_)::const_iterator itr; 
  };

  std::unordered_map<K, Node> k2Node_;


};

TEST(stlList, Simple) {
  std::list<int> l = {
    1, 2, 3, 4
  };
  auto it = l.begin();
  l.splice(l.begin(), l, it);
  EXPECT_EQ(1, *it);
  ++it;
  EXPECT_EQ(2, *it);

  l.splice(l.begin(), l, it);
  EXPECT_EQ(2, *it);
  ++it;
  EXPECT_EQ(1, *it);

  {
    std::list<int> l = {1,};
    auto it = l.begin();
    l.splice(l.begin(), l ,it);
    EXPECT_EQ(1, *it);
    ++it;
    EXPECT_EQ(l.end(), it);
  }
}

TEST_F(LruCacheTest, Simple) {
  {
    SCOPED_TRACE("LruCacheWithPointersAndStdUnorderedSet");
    doTest<LruCacheWithPointersAndStdUnorderedSet>();
  }
  {
    SCOPED_TRACE("LruCacheWithIndexesAndStdUnorderedSet");
    // doTest<LruCacheWithIndexesAndStdUnorderedSet>();
  }

  {
    SCOPED_TRACE("LruCacheList");
    doTest<LruCacheList>();
  }
}



int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
