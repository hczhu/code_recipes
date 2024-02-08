#include "../cpp/header.h"

template<typename T>
class _DisplayType;

template<typename T>
void _displayType(T&& t);

#define PEEK(x) LOG(INFO) << #x << ": [" << (x) << "]"

class Base {
 public:
  virtual ~Base() = default;
  virtual int foo() const {
    LOG(INFO) << "Base::foo() got called.";
    return 0;
  }
};

class Derived : public Base {
 public:
  int foo() const override {
    LOG(INFO) << "Derived::foo() got called.";
    return 1;
  }
};

Base returnValue() {
  return Derived();
}

const Base& returnRef() {
  static Derived dd;
  return dd;
}

TEST(RvaluePolymorphism, Basic) {
  {
    const auto& b = returnValue();
    EXPECT_EQ(b.foo(), 0);
  }
  {
    const auto& b = returnRef();
    EXPECT_EQ(b.foo(), 1);
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

