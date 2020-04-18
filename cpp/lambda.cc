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
#include <mutex>
#include <cstring>
#include <complex>
#include <numeric>
#include <functional>
//#define NDEBUG
#include <cassert>
#include <limits>
//INT_MAX INT_MIN
#include <limits.h>
#include <cstdarg>
#include <iterator>
#include <valarray>
#include <thread>
using namespace std;
#define debug(x) cerr<<#x<<"=\""<<x<<"\""<<" at line#"<<__LINE__<<endl;

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

int main() {
  int thread_num = 10;
  int count = 0;
  vector<thread> threads;
  mutex lock;
  for (int i = 0; i < thread_num; ++i) {
    // Trap! Have to value capture 'i'.
    threads.emplace_back([&, i](){
      lock.lock();
      cout << "counter = " << count++ << " for thread #"  << i << endl;
      lock.unlock();
    });
  }
  for (auto& thr : threads) {
    thr.join();
  }
  vector<int> vect;
  auto func = [&]() {
    vect.push_back(10);
  };
  func();
  std::function<void()> a = {};
  std::function<void()> b = [] {};
  if (a) {
    std::cout << "a is not null";
  }
  if (b) {
    std::cout << "b is not null";
  }
  std::function<void()> c = <::> <%%>;

  // _displayType(a);
  // _displayType(b);
  return 0;
}
