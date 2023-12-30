#include "../header.h"

using namespace std;

class Arena {
public:
  void *get_mem(unsigned int num) { return new char[num]; };
};
class Base {
public:
  virtual int foo() { return 0; };
  void *operator new(size_t num) {
    PEEK(num);
    return ::new char[num];
  };
  Base(int x = -1) : x_(x) { PEEK(x); };

  virtual ~Base() { PEEK(x_); };

 private:
  int x_;
};

class Derived : public Base {
public:
  void *operator new(size_t num, Arena &mem) {
    PEEK(num);
    return mem.get_mem(num);
  }
  Derived(int x) : Base(x) { PEEK(x); }
};
class Simple {
  int a;
};
int main() {
  Arena pool;
  Base *ptr = new Base(100);
  Base *derived = new (pool) Derived(200);
  // placement new
  char buf[sizeof(Simple)];
  Simple *fake_new_ptr = new (buf) Simple();
  return 0;
}
