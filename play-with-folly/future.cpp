#include <iostream>

// #include <glog/logging.h>
#include <gflags/gflags.h>

// #include <folly/executors/IOThreadPoolExecutor.h>
// #include <folly/executors/CPUThreadPoolExecutor.h>
// #include <folly/futures/Future.h>
// #include <folly/Executor.h>

DEFINE_int32(haha, -1, "");

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "haha = " << FLAGS_haha;
  return 0;
}
