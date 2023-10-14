#include <header.h>



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
   BinarySemaphore(const BinarySemaphore &) = delete;
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
       return;
     }
     cv_.wait(lg, [this] { return v_ == 1; });
     v_ = 0;
   }

 private:
  std::mutex m_;
  size_t v_;
  std::condition_variable cv_;
};

#endif

class Semaphore {
 public:
   Semaphore(size_t v = 0) : v_(std::max<size_t>(0, v)) {}
   void release() & {
    {
      std::lock_guard lg(m_);
      ++v_;
    }
    cv_.notify_all();
   }

   //A blocking API
   void acquire() & {
     std::unique_lock lg(m_);
     if (v_ > 0) {
       --v_;
       return;
     }
     cv_.wait(lg, [this] { return v_ > 0; });
     --v_;
   }

   size_t value() const {
     std::lock_guard<std::mutex> lg(m_);
     return v_;
   }

 private:
   mutable std::mutex m_;
   size_t v_;
   std::condition_variable cv_;
};

TEST(BinarySemaphoreTest, Basic) {
  BinarySemaphore bs(0);
  std::atomic<int> v{1};
  std::thread adder([&] { bs.acquire(); ++v; });

  v = v * 2;
  bs.release();
  adder.join();

  EXPECT_EQ(3, v);
}

TEST(BinarySemaphoreTest, Threads) {
  BinarySemaphore bs(1);
  std::atomic<size_t> threadsInCriticalSection{0};
  std::vector<std::thread> threads;
  std::mt19937 rnd(std::time(nullptr));
  std::uniform_int_distribution<size_t> dist(0, 60);
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&] {
      for (int j = 0; j < 10; ++j) {
        std::this_thread::sleep_for(std::chrono::milliseconds(60 + dist(rnd)));
        bs.acquire();
        ++threadsInCriticalSection;
        CHECK_EQ(1, threadsInCriticalSection);
        std::this_thread::sleep_for(std::chrono::milliseconds(60 + dist(rnd)));
        --threadsInCriticalSection;
        bs.release();
      }
    });
  }

  for (auto& thr : threads) {
    thr.join();
  }
}

TEST(SemaphoreTest, Threads) {
  Semaphore bs(3);
  std::atomic<size_t> threadsInCriticalSection{0};
  std::vector<std::thread> threads;
  std::mt19937 rnd(std::time(nullptr));
  std::uniform_int_distribution<size_t> dist(0, 60);
  bool seen3 = false;
  for (int i = 0; i < 15; ++i) {
    threads.emplace_back([&] {
      for (int j = 0; j < 10; ++j) {
        std::this_thread::sleep_for(std::chrono::milliseconds(60 + dist(rnd)));
        bs.acquire();
        ++threadsInCriticalSection;
        CHECK_LE(threadsInCriticalSection, 3);
        if (3 == threadsInCriticalSection) {
          seen3 = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(60 + dist(rnd)));
        --threadsInCriticalSection;
        bs.release();
      }
    });
  }

  for (auto& thr : threads) {
    thr.join();
  }
  EXPECT_TRUE(seen3);
}

class PhilosophersSync {
 public:
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

  std::unique_lock<std::mutex> acquireLock() & {
    return std::unique_lock<std::mutex>(m_);
  }

  void takeForks(size_t pid, std::function<void(size_t)> &cb,
                 std::function<void(size_t, size_t)> &releaseCb) & {
    {
      std::lock_guard<std::mutex> lg(m_);
      states_[pid] = State::HUNGRY;
      tryToEat(pid, [&] { releaseCb(pid, pid); });
    }
    // May be blocked here until both left and right philosophers yield by
    // calling sems_[pid].release() in 'tryToEat(pid)' on behalf of 'pid'.
    sems_[pid]->acquire();
    cb(pid);
  }

  void putForks(size_t pid, std::function<void(size_t)> &cb,
                std::function<void(size_t, size_t)> &releaseCb) & {
    std::lock_guard<std::mutex> lg(m_);
    states_[pid] = State::THINKING;
    cb(pid);
    tryToEat((pid + 1) % numPhils_,
             [&, this] { releaseCb((pid + 1) % numPhils_, pid); });
    tryToEat((pid + numPhils_ - 1) % numPhils_,
             [&, this] { releaseCb((pid + numPhils_ - 1) % numPhils_, pid); });
  }

  std::vector<State> getStates() const {
    std::vector<State> res;
    {
      std::lock_guard lg(m_);
      res = states_;
    }
    return res;
  }

 private:
  const size_t numPhils_;
  mutable std::mutex m_;
  std::vector<State> states_;
  std::vector<std::unique_ptr<BinarySemaphore>> sems_;

  // The caller must be holding 'm_'.
  // It's a non-blocking API.
  void tryToEat(size_t pid, std::function<void()> &&cb) & {
    if (states_[pid] == State::HUNGRY &&
        states_[(pid + numPhils_ - 1) % numPhils_] != State::EATING &&
        states_[(pid + 1) % numPhils_] != State::EATING) {
      states_[pid] = State::EATING;
      // The beauty of this algorithm such that another philosopher can release
      // the semaphore for 'pid' and starvation can be avoided by design.
      sems_[pid]->release();
      if (cb) {
        cb();
      }
    }
  }
};

TEST(DiningPhilosophersTest, Basic) {
  std::vector<size_t> thinkingTime = {
      0,  // The fastest
      20, // Slow
      1,
      40, // Super slow
      1,
  };
  std::vector<size_t> eatingTime = {
      0,  // The fastest
      5,  // Slow
      1,
      10, // Super slow
      1,
  };
  const size_t n = 5;
  const size_t runTimeSec = 5;
  std::mutex m;
  using P = std::pair<size_t, size_t>;
  std::vector<P> seq;
  std::function<void(size_t)> takeForks = [&](size_t pid) {
    std::lock_guard lg(m);
    seq.push_back(P(pid + 1, 0));
  };
  std::function<void(size_t)> putForks = [&](size_t pid) {
    std::lock_guard lg(m);
    seq.push_back(P(0, pid + 1));
  };

  std::function<void(size_t, size_t)> release = [&](size_t pid,
                                                    size_t callerPid) {
    std::lock_guard lg(m);
    seq.push_back(P(pid + 1, callerPid + 1));
  };

  auto checkActions = [&] {
    std::vector<bool> forksInUse(5, false);
    for (auto pr : seq) {
      auto p1 = pr.first;
      auto p2 = pr.second;
      if (p2 == 0) {
        int l = p1 -1;
        int r = (l + 1) % n;
        // std::cout << "#" << (p1 - 1) << " picked up forks " << l << " and " << r << std::endl;
        CHECK(!forksInUse[l]);
        CHECK(!forksInUse[r]);
        forksInUse[l] = forksInUse[r] = true;
      } else if (p1 == 0) {
        int l = p2 - 1;
        int r = (l + 1) % n;
        // std::cout << "#" << (p2 - 1) << " put forks " << l << " and " << r << std::endl;
        CHECK(forksInUse[l]);
        CHECK(forksInUse[r]);
        forksInUse[l] = forksInUse[r] = false;
      } else {
        // std::cout << "#" << (p2 - 1) << " released " << "#" << (p1 - 1) << std::endl;
      }
    }
  };

  auto runTest = [&] {
    PhilosophersSync ps(n);

    std::mt19937 rnd(std::time(nullptr));
    std::uniform_int_distribution<size_t> dist(0, 30);

    std::atomic<size_t> numPhilDone{n};

    auto getEpoch = [] { return std::time(nullptr); };

    auto startPhilosopher = [&](const size_t pid, const size_t thinkingTime,
                                const size_t eatingTime) {
      const auto startEpoch = getEpoch();
      size_t r = 0;
      while (getEpoch() < startEpoch + runTimeSec) {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(thinkingTime + dist(rnd)));
        ps.takeForks(pid, takeForks, release);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(eatingTime + dist(rnd)));
        ps.putForks(pid, putForks, release);
        ++r;
        // LOG(INFO) << "#" << pid << " finished round #" << r << " @"
                  // << getEpoch();
      }
      --numPhilDone;
      return r;
    };
    std::vector<size_t> howManyTimesEating(n, 0);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < n; ++i) {
      threads.emplace_back([&, pid = i] {
        howManyTimesEating[pid] =
            startPhilosopher(pid, thinkingTime[pid], eatingTime[pid]);
      });
    }

    while (numPhilDone > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(3));
      const auto st = ps.getStates();
      for (size_t pid = 0; pid < n; ++pid) {
        if (st[pid] == PhilosophersSync::State::EATING) {
          CHECK(st[(pid + 1) % n] != PhilosophersSync::State::EATING);
        }
      }
    }

    for (auto &thr : threads) {
      thr.join();
    }

    LOG(INFO) << "-----------------";
    for (size_t pid = 0; pid < n; ++pid) {
      LOG(INFO) << "Philosopher #" << pid << " ate " << howManyTimesEating[pid]
                << " times.";
    }

    LOG(INFO) << "There are " << seq.size() << " actions.";
    checkActions();
  };

  SCOPED_TRACE("5 philosophers");
  runTest();

  SCOPED_TRACE("2 philosophers");
  seq.clear();
  thinkingTime[1] = thinkingTime[2] = thinkingTime[4] = runTimeSec * 1000 + 200;
  runTest();
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

