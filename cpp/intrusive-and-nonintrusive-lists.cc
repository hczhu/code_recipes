#include "../cpp/header.h"

using namespace std;

class Link {
  public:
    Link(Link* next = NULL) : next_(next) {}
    Link*& Next() {
      return next_;
    }
  protected:
    Link *next_;
};

class IntrusiveList {
  public:
    void AddToFront(Link* node) {
      node->Next() = head_.Next();
      head_.Next() = node;
    }
    Link* Start() {
      return head_.Next();
    }
  private:
    Link head_;
};

class Node : public Link {
  public:
    Node(int v = 0) : data_(v) {}
  private:
    int data_;
};

template<class T>
class NonIntrusiveList {

  class InnerNode : public Link {
    public:
      InnerNode(const T* user_data) : user_data_(user_data) {}
      const T* user_data_;
  };
  IntrusiveList list_;
  public:
    void AddToFront(const T& new_data) {
      list_.AddToFront(new InnerNode(&new_data));
    }
};

#define CHECK_DERIVE(x, y) do { y* _tmp_y = (x*)(0); _tmp_y = NULL;} while(0);

int main() {
  CHECK_DERIVE(Node, Link);
  const int n = 100000;
  double start = 1.0 * clock() / CLOCKS_PER_SEC;
  IntrusiveList intrusive_list;
  for (auto i : std::views::iota(0, n)) {
    intrusive_list.AddToFront(new Node(i));
  }
  cerr << "Intrusive Time = " << 1.0 *clock() / CLOCKS_PER_SEC - start << " s" << endl;
  start = 1.0 * clock() / CLOCKS_PER_SEC;
  NonIntrusiveList<int> nonintrusive_list;
  for (auto i : std::views::iota(0, n)) {
    nonintrusive_list.AddToFront(*(new int(i)));
  }
  cerr << "Non-intrusive Time = " << 1.0 *clock() / CLOCKS_PER_SEC - start << " s" << endl;
  return 0;
}
