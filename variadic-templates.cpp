#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <iostream>
#include <cmath>
#include <vector>
#include <queue>
#include <set>
#include <sstream>
#include <map>
#include <cstring>
#include <complex>
#include <numeric>
#include <functional>
#include <cassert>
#include <limits>
#include <limits.h>
#include <cstdarg>
#include <iterator>
#include <valarray>
#include <thread>
#include <memory>
using namespace std;

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>

#define debug(x) cerr<<#x<<"=\""<<x<<"\""<<" at line#"<<__LINE__<<endl;

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

template<typename Policy>
void print(Policy& outputStream) {
  outputStream << std::endl;
}

template<typename... Args>
using VT = int;

template<typename Policy, typename Arg1, typename... Args>
void print(Policy& outputStream, const Arg1& arg1, const Args&... args) {
  outputStream << arg1 << " ";
  print(outputStream, args...);
}

template<typename... Args>
void confirm(const Args&... msg) {
  int _[] = {
    (std::cout << msg << " ", 0)...
  };
  std::cout << " N/y?" << std::endl;
  (void)_;
}

struct Vec {
  template<typename... Args>
  static std::shared_ptr<Vec> make(Args&&... eles) {
    auto res = std::make_shared<Vec>();
    res->add(std::forward<Args>(eles)...);
    return res;
  }
  template<typename Arg1, typename... Args>
  void add(Arg1 ele, Args... eles) {
    add(ele);
    add(eles...);
  }
  
  template<typename Arg>
  void add(Arg ele) {
    vec_.push_back(int(ele));
  }

  std::vector<int> vec_;
};

int foo(int) {
  return -1;
}

template<class... Container>
bool anyEmptyContainer(const Container&... args) {
  return (...|| args.empty());
}

TEST(Main, VariadicUsing) {
  static_assert(std::is_same_v<VT<int, double, std::string>, int>);
}

TEST(Main, Main) {
  auto& t = foo;
  print(cout, 1, 2, "sfsfd", 1.34243, 'a', 3241234);
  string ss;
  stringstream os(ss);
  print(os, 1, 2, "sfsfd", 1.34243, 'a', 3241234);
  cout << os.str() << std::endl;

  auto vec = Vec::make(1, 2, 4, 5);
  print(cout,
    vec->vec_[0],
    vec->vec_[1],
    vec->vec_[2],
    vec->vec_[3]
  );
  confirm(1, 2, "aaa", 1.2, 1.5, -1);
}

TEST(Variadic, IfExpansion) {
  EXPECT_TRUE(anyEmptyContainer(std::string(), std::string("dfa")));
  EXPECT_FALSE(anyEmptyContainer(std::vector<int>({
                                     1, 2,
                                 }),
                                 std::vector<int>({
                                     3,
                                 })));

  EXPECT_TRUE(anyEmptyContainer(std::vector<int>({}),
                                std::vector<int>({
                                    3,
                                })));
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
