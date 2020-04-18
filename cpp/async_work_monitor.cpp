
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

#include <folly/experimental/FunctionScheduler.h>

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"


class AsyncWorkMonitor {
 public:
  using AsyncWorkId = size_t;

  using Progress = int;
  constexpr Progress kDone = 0;

  constexpr AsyncWorkId kInvalidAsyncWorkId = 0;

  struct Options {
    std::chrono::milliseconds slow_progress_threshold{10 * 1000};
    // Won't run the background thread if = 0.
    std::chrono::milliseconds monitor_thread_run_interval{0};
  };

  explicit AsyncWorkMonitor(const Options& options = Options{});

  // Return a new rpc sequence id.
  // If the returned id == kInvalidRpcId, don't use it.
  AsyncWorkId startAsyncWork(const std::string &asynWorkInfo,
                             const Progress startProgress);

 protected:
  void runMonitorOnceNoException() noexcept;

 private:
  void runMonitorOnce();

  struct MonitorSlot {
    std::atomic<Progress> progress{kDone};
    std::string asyncWorkInfo;
    std::chrono::steady_clock::time_point lastProgress;
  };

  const Options options_;
  constexpr size_t kNumSlots = 110'273;
  std::array<MonitorSlot, kNumSlots> slots_;

  std::atomic<AsyncWorkId> sequenceId_{1};
  folly::FunctionScheduler functionScheduler_;
};

AsyncWorkMonitor::AsyncWorkMonitor(const Options &options = Options{})
    : options_(options) {
  if (options_.monitor_thread_run_interval.count() > 0) {
    LOG(INFO) << "Startign the monitoring thread to run every "
              << options_.monitor_thread_run_interval;
    functionScheduler_.setSteady(true);
    functionScheduler_.addFunction([this] { runMonitorOnceNoException(); },
                                   options_.monitor_thread_run_interval,
                                   "AsyncMon");
    functionScheduler_.setThreadName("AsyncMon");
  }
}

AsyncWorkId AsyncWorkMonitor::startAsyncWork(const std::string &asynWorkInfo,
                                             const Progress startProgress) {
  const auto id = sequenceId_++;
  Progress expected = kDone;
  auto &slot = slots_[rpcId % slots_.size()];
  if (!std::atomic_compare_exchange_strong(
          &slot.progress, &expected, startProgress)) {
    // This slot is already occupied. Reject monitoring this async work.
    return kInvalidAsyncWorkId;
  }
  slot.asynWorkInfo = asynWorkInfo;
  slot.lastProgress = std::chrono::steady_clock::now();
  return id;
}

TEST(FooTest, Bar) {
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

