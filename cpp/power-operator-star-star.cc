// Overload of '*' to form a new operator '**' to implement rising to power.
#include "../header.h"
using namespace std;
#define debug(x) cerr<<#x<<"=\""<<x<<"\""<<" at line#"<<__LINE__<<endl;
#define Rep(i,n) for(int i=0;i<(n);++i)
#define For(i,a,b) for(typeof(a) i=a;i<(b);++i)
#define SZ(vec) ((int)(vec).size())
#define All(vec) (vec).begin(),(vec).end()

class Index {
  private:
    struct inner_class {int value;};
  public:
    Index(int idx = 0) : idx_(idx) {}
    Index(const inner_class&  inner) : idx_(inner.value) {}
    operator int() const {return idx_;}
    inner_class operator*() {
      inner_class inner;
      inner.value = idx_;
      return inner;
    }
    friend double operator*(double, const inner_class& inner);
  private:
    int idx_;
};



double mypow(double base, Index idx) {
  return pow(base, int(idx));
}

double operator*(double base, const Index::inner_class& inner) {
  return mypow(base, Index(inner));
}

int main() {
  vector<int> va;
  for (auto a : va) {
    ++a;
  }
  debug(mypow(1.08,9));
  Index index(9);
  debug((1.08**index));
  debug((1.08**Index(18)));
  return 0;
}
