
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

#include <folly/Demangle.h>
#include <glog/logging.h>
#include <gflags/gflags.h>

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

/* template end */

class ClassNameTrait {
  public:
    ClassNameTrait()
      : getName_(
          [this] { return folly::demangle(typeid(*this)).toStdString(); }),
      name_(typeid(*this).name()) {
        PEEK(name_);
     };

    std::string getName() const {
      return getName_();
    }
    // virtual is important. If not used, the types are not polymorphic.
    virtual ~ClassNameTrait() = default;
  private:
    std::string name_;
    std::function<std::string()> getName_;
};

class MyClass : public ClassNameTrait {
  
};

template<typename T>
class TClass : public ClassNameTrait {

};

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  PEEK(MyClass().getName());
  MyClass my;
  const ClassNameTrait& cnt = my;
  PEEK(cnt.getName());
  PEEK(my.getName());
  PEEK(TClass<int>().getName());
  return 0;
}

