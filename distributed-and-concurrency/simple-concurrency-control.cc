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

// https://en.cppreference.com/w/cpp/thread
#include <latch>
#include <functional>

using namespace std;

class OrderedSync {
public:
  OrderedSync(bool use_atomic = false) : use_atomic_(use_atomic), l1(1), l2(1) {
    LOG(INFO) << "Initial value: " << st;
  }

  void atomicTransit(int expected, int desired) {
    const auto exp = expected;
    while (!std::atomic_compare_exchange_strong(&st, &expected, desired)) {
      expected = exp;
    }
  }

  void first(function<void()> printFirst) {
    // printFirst() outputs "first". Do not change or remove this line.
    printFirst();
    if (use_atomic_) {
      atomicTransit(0, 1);
    } else {
      l1.count_down();
    }
  }

  void second(function<void()> printSecond) {
    if (use_atomic_) {
      atomicTransit(1, 2);
    } else {
      l1.wait();
    }
    // printSecond() outputs "second". Do not change or remove this line.
    printSecond();
    if (use_atomic_) {
      atomicTransit(2, 3);
    } else {
      l2.count_down();
    }
  }

  void third(function<void()> printThird) {
    if (use_atomic_) {
      atomicTransit(3, 4);
    } else {
      l2.wait();
    }
    // printThird() outputs "third". Do not change or remove this line.
    printThird();
  }
  bool use_atomic_;
  std::latch l1, l2;
  std::atomic<int> st {0};
};

TEST(OrderedSyncTest, Simple) {
  auto oneTest = [](OrderedSync &foo) {
    std::mutex mt;
    std::vector<int> output;
    auto getFn = [&](int v) {
      return [&, v] {
        std::lock_guard l(mt);
        LOG(INFO) << "Output " << v;
        output.push_back(v);
      };
    };

    std::thread t3([&]() { foo.third(getFn(3)); });
    std::thread t2([&]() { foo.second(getFn(2)); });
    std::thread t1([&]() { foo.first(getFn(1)); });

    t1.join();
    t2.join();
    t3.join();

    EXPECT_EQ(std::vector<int>({1, 2, 3}), output);
  };

  {
    OrderedSync foo;
    oneTest(foo);
  }
  {
    OrderedSync foo(true);
    oneTest(foo);
  }
}

// Threads will be allowed to hold the lock in the oder of their ids from
// smallest to the largest one repeatedly.
class QueueSync {
 public:
  // Thread id should be in [0, n - 1).
  QueueSync(size_t n) : n_(n) { }

  void run(size_t id, std::function<void()> &&fn) {
    LOG(INFO) << "Running id: " << id;
    std::unique_lock<std::mutex> lg(m_);
    cv_.wait(lg, [this, id] { return nextId_ == id; });
    fn();
    nextId_ = (nextId_ + 1) % n_;
    cv_.notify_all();
    LOG(INFO) << "Finished running id: " << id << " next: " << nextId_;
  }

 private:
  std::mutex m_;
  std::condition_variable cv_;
  size_t nextId_ = 0;
  const size_t n_;
};

TEST(QueueSyncTest, basic) {
  std::vector<std::thread> threads;
  size_t n = 4;
  std::vector<int> output;
  QueueSync qs(n);
  for (size_t id = 0; id < n; ++id) {
    threads.emplace_back([&, n, id] {
      for (int j = 0; j < 3; ++j) {
        qs.run(id, [&, n, j, id] { output.push_back(j * n + id); });
      }
    });
  }

  for (auto& thr : threads) {
    thr.join();
  }

  EXPECT_EQ(output, std::vector<int>({
                        0,
                        1,
                        2,
                        3,
                        4,
                        5,
                        6,
                        7,
                        8,
                        9,
                        10,
                        11,
                    }));
}



int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

