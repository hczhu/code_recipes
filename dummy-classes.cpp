#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <iostream>
#include <cmath>
#include <array>
#include <vector>
#include <queue>
#include <set>
#include <sstream>
#include <map>
#include <cstring>
#include <complex>
#include <numeric>
#include <functional>
#include <cassert>
#include <limits>
#include <limits.h>
#include <cstdarg>
#include <iterator>
#include <valarray>
#include <thread>
#include <memory>
//using namespace std;
#define debug(x) cerr<<#x<<"=\""<<x<<"\""<<" at line#"<<__LINE__<<endl;
#define flag(x) FLAGS_##x

class Base {
 public:
  Base() {
    std::cout << "ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Base(const Base&) {
    std::cout << "Copy ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Base(Base&&) {
    std::cout << "Move ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  ~Base() {
    std::cout << "Dtor in " << __FUNCTION__ << " @" << this << std::endl;
  }
};

class Derived : public Base {
 public:
  Derived() {
    std::cout << "ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Derived(const Base&) {
    std::cout << "Copy ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Derived(Base&&) {
    std::cout << "Move ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  ~Derived() {
    std::cout << "Dtor in " << __FUNCTION__ << " @" << this << std::endl;
  }
};

class Container {
 public:
  Container() {
    std::cout << "ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Container(const Base&) {
    std::cout << "Copy ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  Container(Base&&) {
    std::cout << "Move ctor in " << __FUNCTION__ << " @" << this << std::endl;
  }
  ~Container() {
    std::cout << "Dtor in " << __FUNCTION__ << " @" << this << std::endl;
  }
 private:
  Base b_;
  Derived d_;
};

int main() {
  Container con;
  return 0;
}
