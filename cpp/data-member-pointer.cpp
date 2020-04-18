
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits.h>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <valarray>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

struct Data {
  std::string a;
  int b;
  struct Inner {
    bool c;
    bool d;
  };
  Inner inner;
};

template<class T, typename F>
auto get_lambda(F T::*field) {
  return [=](const T& t) -> F{
    return t.*field;
  };
}

int main(int argc, char* argv[]) {
  auto ptr = &Data::Inner::c;
  Data d;
  d.inner.c = true;
  d.b = -1;
  PEEK(d.inner.*ptr);

  auto lm = get_lambda(&Data::b);

  EXPECT_EQ(-1, lm(d));

  // _DisplayType<decltype(&Data::Inner::c)> _a;
  return 0;
}

