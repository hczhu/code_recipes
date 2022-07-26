#include <utility>
#include <unordered_set>
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "message_queue.h"

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

#define _STR(x) #x
#define STR(x) _STR(x)
#define PRINT_MACRO(M) static_assert(0, "Macro " #M " = " STR(M))

// #define x 42
// PRINT_MACRO(x);

/* template end */
void relaxedMagic1() {
  std::atomic<int> x(0), y(0);
  std::atomic<bool> loaded{false};
  bool stop = false;
  std::mutex mt;
  std::condition_variable cv;
  std::thread writer([&]{
    while (!stop) {
      x.store(1,std::memory_order_relaxed);
      y.store(1,std::memory_order_relaxed);
      std::unique_lock<std::mutex> lk(mt);
      cv.wait(lk, [&]{
        return stop || loaded;
      });
      loaded = false;
    }
  });

  std::thread reader([&]{
    int round = 0;
    while (!stop) {
      int a = y.load(std::memory_order_relaxed);
      int b = x.load(std::memory_order_relaxed);
      std::cout << "Round #" << ++round << std::endl;
      if (a==1 && b!=1) {
        std::cout << "Magic happend: a == 1 && b != 1." << std::endl;
        stop = true;
      }
      loaded = true;
      cv.notify_one();
    }
  });

  reader.join();
  writer.join();
}

int main() {
  relaxedMagic1();
  return 0;
}
