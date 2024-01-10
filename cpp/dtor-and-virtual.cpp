#include "../cpp/header.h"


class Base1 {
 public:
  virtual ~Base1() {
    haha();
    foo();
  }
 private:
  void haha() {
    foo();
  }
  virtual void foo() {
    LOG(INFO) << "Called Base::foo()";
  }
};

class Derived1 : public Base1 {
 public:
  ~Derived1() {
    foo();
  }
 private:
  void foo() override { LOG(INFO) << "Called Derived::foo()"; }
};

namespace multi_thr {

class Base {
 public:
   void startThr() {
     bgThr_ = std::thread([this] {
       while (!stop_.load())
         foo();
     });
   }
   virtual ~Base() {
     // std::this_thread::sleep_for(std::chrono::seconds(3));
     stop_.store(true);
     bgThr_.join();
  }

 private:
  virtual void foo() = 0;
  std::thread bgThr_;
  std::atomic<bool> stop_{false};
};

class Derived : public Base {
 public:
  ~Derived() {
    LOG(INFO) << "Called Derived::~Derived()";
  }
 private:
  void foo() override {
    LOG(INFO) << "Calling Derived::foo()";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
};

}

TEST(Foo, Bar) {
  // Derived1 dd;
  multi_thr::Derived dev;
  dev.startThr();
  std::this_thread::sleep_for(std::chrono::seconds(6));
}


int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

