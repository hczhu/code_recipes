
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

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

/* template end */

struct Foo : public std::enable_shared_from_this<Foo> { };

class A {

};

class B : public A {

};

void foo(std::shared_ptr<A>) {

}

TEST(Foo, Bar) {
  foo(std::make_shared<B>());
  {
    EXPECT_EQ(1, 1);
    auto p1 = std::make_shared<Foo>();
    EXPECT_EQ(1, p1.use_count());
    auto p2 = p1;
    EXPECT_EQ(2, p1.use_count());
    
    auto rawPtr = p1.get();
    std::shared_ptr<Foo> p3 = rawPtr->shared_from_this();
    EXPECT_EQ(3, p1.use_count());
    EXPECT_EQ(3, p3.use_count());
  }
  {
    EXPECT_EQ(1, 1);
    auto p1 = std::make_shared<std::string>();
    EXPECT_EQ(1, p1.use_count());
    auto p2 = p1;
    EXPECT_EQ(2, p1.use_count());
    /*
    std::shared_ptr<std::string> p3(p1.get());
    EXPECT_EQ(2, p1.use_count());
    EXPECT_EQ(1, p3.use_count());
    */
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

