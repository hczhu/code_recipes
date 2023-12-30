#include "../header.h"

using namespace std;
#define debug(x) cerr<<#x<<"=\""<<x<<"\""<<" at line#"<<__LINE__<<endl;

class Key {
 public:
  int a;
  Key() {
    cout << "Key ctor." << endl;
  }
  Key(const Key& other) {
    a = other.a;
    cout << "Key copy ctor." << endl;
  }
  bool operator==(const Key& other) const {
    return a == other.a;
  }
};

template <>
struct std::hash<Key> {
  size_t operator()(const Key& key) const {
    return key.a;
  }
};

int main() {
  unordered_map<Key, int> umap;
  Key k;
  k.a = 0; umap[k] = k.a;
  k.a = 1; umap[k] = k.a;
  k.a = 2; umap[k] = k.a;
  auto itr = umap.find(k);
  cout << itr->first.a << " " << itr->second << endl;
  return 0;
}
