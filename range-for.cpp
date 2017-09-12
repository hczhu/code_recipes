#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <iostream>
#include <cmath>
#include <vector>
#include <queue>
#include <set>
#include <sstream>
#include <map>
#include <cstring>
#include <complex>
#include <numeric>
#include <functional>
//#define NDEBUG
#include <cassert>
#include <limits>
//INT_MAX INT_MIN
#include <limits.h>
#include <cstdarg>
#include <iterator>
#include <valarray>
#include <thread>

#include "temp-util.h"
#include "dummy-classes.h"

using namespace std;

template<typename T>
void foo(T&& bases) {
  // DisplayType<decltype(bases)> disp1;
  for (auto b : bases) {
    // DisplayType<decltype(b)> disp1;
    b.print();
  }
}

void mapFor(const std::map<std::string, std::string>& mp) {
  for (auto& pr : mp) {
    DisplayType<decltype(pr)> disp;
  }
}

int main() {
  // foo(std::vector<Base>());
  std::vector<Base> bases(3);
  // const auto& ref = bases;
  debug("-----");
  foo(bases);

  std::string haha;
  auto& hf = haha;

  DisplayType<decltype(hf)> disp;

  std::unique_ptr<int> intp(new int(10));
  std::vector<decltype(intp)> intv(1, std::move(intp));

  return 0;
}
