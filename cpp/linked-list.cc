#include "../cpp/header.h"

using namespace std;

template <typename T>
class Link {
 public:
  Link(T* next = nullptr) : next_(next) {}
  T*& next() { return next_; }

 protected:
  T* next_;
};

class Node : public Link<Node> {
 public:
  Node(int v = 0, Node* next = nullptr) : Link<Node>(next), data_(v) {}
  int value() const { return data_; }

 private:
  int data_;
};

Node* fromVector(const vector<int>& vec) {
  Node head;
  Node* tail = &head;

  for (int v : vec) {
    Node* node = new Node(v);
    tail->next() = node;
    tail = node;
  }
  return head.next();
}

vector<int> toVector(Node* head) {
  vector<int> vec;
  while (head) {
    vec.push_back(head->value());
    head = head->next();
  }
  return vec;
}

Node* swapEvery2(Node* head) {
  if (head == nullptr) {
    return nullptr;
  }
  Node sentinel(0, head);
  auto p0 = &sentinel;
  auto p1 = head;
  auto p2 = head->next();
  while (p1 && p2) {
    p1->next() = p2->next();
    p2->next() = p1;
    p0->next() = p2;

    p0 = p1; // p1 is the new p2
    p1 = p0->next();
    if (p1) {
      p2 = p1->next();
    }
  }
  return sentinel.next();
}

TEST(LinkedList, SwapEvery2) {
  vector<int> all = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  for (size_t l = 0; l < all.size(); ++l) {
    SCOPED_TRACE("Test case with vector length: " + std::to_string(l));
    auto vec = vector<int>(all.begin(), all.begin() + l);
    auto head = fromVector(vec);
    auto swapped = swapEvery2(head);
    auto swappedVec = toVector(swapped);
    auto expected = vector<int>(vec);
    for (size_t i = 0; i + 1 < expected.size(); i += 2) {
      swap(expected[i], expected[i + 1]);
    }
    EXPECT_EQ(expected, swappedVec) << "l = " << l;
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
