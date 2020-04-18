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
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <limits.h>
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

#include <glog/logging.h>
#include <gflags/gflags.h>

#include <folly/Range.h>
#include <folly/FixedString.h>

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

/* template end */

// const folly::StringPiece
inline folly::StringPiece constexpr spVariable() {
  return "const-sp";
}

// const foly::FixedString.
inline auto constexpr prefix() {
  return  folly::makeFixedString("prefix");
}

inline auto constexpr constFS() {
  return prefix() + folly::makeFixedString(".suffix");
}

// const string
static auto const gV = std::string_literals::operator""s("const-string", 11);

using namespace std::string_literals;
static auto const variable = "const-string"s;

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  // _displayType(derived());
  PEEK(spVariable());
  PEEK(prefix());
  PEEK(constFS());
  PEEK(gV);
  PEEK(variable);
  return 0;
}
