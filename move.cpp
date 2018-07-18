
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
#include <cassert>
#include <limits>
#include <limits.h>
#include <cstdarg>
#include <iterator>
#include <valarray>
#include <thread>
#include <memory>

#include "temp-util.h"
#include "dummy-classes.h"

template <typename T>                                                                                                         
constexpr typename std::decay<T>::type copy(T&& value) noexcept(                                                              
    noexcept(typename std::decay<T>::type(std::forward<T>(value)))) {
  return std::forward<T>(value);
}

template <typename T>
typename std::remove_reference<T>::type&& move(const T&& v) = delete;

template <typename T>
typename std::remove_reference<T>::type&& move(T&& v) noexcept {
  return static_cast<typename std::remove_reference<T>::type&&>(v);
}



int main() {
  Base b;
  std::pair<const Base, Base> pr(b, std::move(b));
  return 0;
  const Base aa;
  // DisplayType<decltype(std::move(aa))> dt;
  auto bb = move(aa);
  return 0;
}
