#include "../cpp/header.h"


using func_ref_t = int(&)();

int forward(func_ref_t f) {
  return f();
}

int foo() {
  return -1;
}

TEST(Foo, Bar) {
  EXPECT_EQ(-1, forward(foo));
  // EXPECT_EQ(1, forward([] { return 1; }));
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

