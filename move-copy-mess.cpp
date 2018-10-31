
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
#include <gtest/gtest.h>

#include "dummy-classes.h"

/* template end */


TEST(Foo, Bar) {
  struct Container {
    Container() = default;
    Container(const std::string c) : a(c) { }
    const std::string a = "aaa";
    Base b;
    const Base d;
  };

  Container a("haha");
  Container b(std::move(a));
  EXPECT_EQ(1, Base::MoveCnt);
  EXPECT_EQ(1, Base::CopyCnt);
  EXPECT_EQ("haha", a.a);
  EXPECT_EQ("haha", b.a);

  auto c = std::move(b);
  EXPECT_EQ(2, Base::MoveCnt);
  EXPECT_EQ(2, Base::CopyCnt);
  EXPECT_EQ("haha", c.a);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

