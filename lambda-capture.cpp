
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
#include <gtest/gtest.h>

#include "dummy-classes.h"

/* template end */

TEST(Foo, Bar) {
  Base a;
  auto f = [aa = std::move(a)]() mutable {
    std::cout << "Move lambda capture." << std::endl;
    aa.set(-100);
    aa.print();
  };
  std::cout << "---------------" << std::endl;
  auto f1 = [b = a] {
    std::cout << "Copy lambda capture." << std::endl;
    b.print();
  };
  std::cout << "---------------" << std::endl;
  f();
  f1();
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

