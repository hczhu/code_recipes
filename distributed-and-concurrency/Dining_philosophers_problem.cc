
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


// https://www.wikiwand.com/en/Dining_philosophers_problem
//
#if __cplusplus >= 202002L
// C++20 (and later) code
#include <semaphore>
using BinarySemaphore = std::binary_semaphore;
#else

class BinaySemaphore {
 public:
   BinarySemaphore(size_t v) v_(std::min(1, std::max(0, v))) {}
   void release() & {
    {
      std::lock_guard lg(m_);
      v_ = 1;
    }
    cv_.notify_all();
   }

   //A blocking API
   void acquire() & {
     std::unique_lock lg(m_);
     if (v_ == 1) {
       v_ = 0;
     } else {
       cv_.wait(lg, [this] { return v_ == 1; });
     }
   }

 private:
  std::mutex m_;
  size_t v_;
  std::condition_variable cv_;

};

#endif

TEST(FooTest, Bar) {
  BinarySemaphore bs(0);
  std::atomic<int> v{1};
  std::thread adder([&] { bs.acquire(); ++v; });

  v = v * 2;
  bs.release();
  adder.join();

  EXPECT_EQ(3, v);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

