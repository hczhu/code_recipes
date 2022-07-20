
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
#if __cplusplus >= 202002L and 0
// C++20 (and later) code
#include <semaphore>
using BinarySemaphore = std::binary_semaphore;
#else

class BinarySemaphore {
 public:
   BinarySemaphore(size_t v = 0)
       : v_(std::min<size_t>(1, std::max<size_t>(0, v))) {}
   BinarySemaphore(const BinarySemaphore &) = default;
   void release() & {
    {
      std::lock_guard lg(m_);
      v_ = 1;
    }
    cv_.notify_one();
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

TEST(BinarySemaphoreTest, Basic) {
  BinarySemaphore bs(0);
  std::atomic<int> v{1};
  std::thread adder([&] { bs.acquire(); ++v; });

  v = v * 2;
  bs.release();
  adder.join();

  EXPECT_EQ(3, v);
}

struct PhilosophersSync {
  enum class State {
    THINKING = 0, // philosopher is thinking
    HUNGRY = 1,   // philosopher is trying to get forks
    EATING = 2,   // philosopher is eating
  };

  PhilosophersSync(size_t n)
      : numPhils_(n), states_(n, State::THINKING) {
    for (size_t i = 0; i < numPhils_; ++i) {
      sems_.push_back(std::make_unique<BinarySemaphore>(0));
    }
  }

  // The caller must be holding 'm_'.
  // It's a non-blocking API.
  void tryToEat(size_t pid) & {
    if (states_[pid] == State::HUNGRY
        && states_[(pid + numPhils_ - 1) % numPhils_] != State::EATING
        && states_[(pid + 1) % numPhils_] != State::EATING) {
      states_[pid] = State::EATING;
      // The beauty of this algorithm such that another philosopher can release
      // the semaphore for 'pid' and starvation can be avoided by design.
      sems_[pid]->release();
    }
  }

  std::unique_lock<std::mutex> acquireLock() & {
    return std::unique_lock<std::mutex>(m_);
  }

  void takeForks(size_t pid, std::function<void()> cb) & {
    {
      std::lock_guard<std::mutex> lg(m_);
      states_[pid] = State::HUNGRY;
      tryToEat(pid);
    }
    // May be blocked here until both left and right philosophers yield by
    // calling sems_[pid].release() in 'tryToEat(pid)' on behalf of 'pid'.
    sems_[pid]->acquire();
    cb();
  }

  void putForks(size_t pid) & {
    std::lock_guard<std::mutex> lg(m_);
    states_[pid] = State::THINKING;
    tryToEat((pid + 1) % numPhils_);
    tryToEat((pid + numPhils_ - 1) % numPhils_);
  }

 private:
  const size_t numPhils_;
  std::mutex m_;
  std::vector<State> states_;
  std::vector<std::unique_ptr<BinarySemaphore>> sems_;
};

TEST(DiningPhilosophersTest, Basic) {

}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

