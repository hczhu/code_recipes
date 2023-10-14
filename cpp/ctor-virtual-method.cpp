#include "../header.h"


class Base {
 public:
  Base(Base* b = nullptr) : base_(b ? b : this) { }
  virtual ~Base() = default;
  virtual int callFoo() {
    return base_->foo();
  }
  virtual int foo() {
    return 0;
  }
  Base* base_ = nullptr;
};

class Derived : public Base {
 public:
  Derived() : Base(this) { }
  int foo() {
    return 1;
  }
};

TEST(Foo, Bar) {
  Derived dd;
  EXPECT_EQ(1, dd.callFoo());
  Base* bb = &dd;
  EXPECT_EQ(1, bb->callFoo());
  EXPECT_EQ(0, Base().callFoo());
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

