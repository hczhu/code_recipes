#include "../header.h"
using namespace std;

void throw_foo(int a) { throw a; }

int return_foo(int a) { return a; }

int main() {
  double start = 1.0 * clock() / CLOCKS_PER_SEC;
  for (auto x : std::views::iota(0, 1000000)) {
    try {
      throw_foo(x);
    } catch (...) {
    }
  }
  cerr << (1.0 * clock() / CLOCKS_PER_SEC - start) << endl;

  start = 1.0 * clock() / CLOCKS_PER_SEC;
  for (auto x : std::views::iota(0, 1000000)) {
    return_foo(x);
  }
  cerr << (1.0 * clock() / CLOCKS_PER_SEC - start) << endl;
  return 0;
}
