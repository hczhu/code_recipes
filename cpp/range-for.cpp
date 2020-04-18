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
void _displayType(T&& t);

void bar(std::map<int, std::string>&& mp) {
  for (auto& pr : mp) {
    _displayType(pr);
  }
}

int main() {
  std::map<int, std::string> mp;
  bar(std::move(mp));
  return 0;
}
