#include "../cpp/header.h"


TEST(FooTest, Bar) {
  int a = 100;
  auto fib = [&](int n, auto &&fib) -> int {
    if (n <= 1)
      return n;
    return fib(n - 1, fib) + fib(n - 2, fib);
  };
  auto runFib = [&](int n) {
    return fib(n, fib);
  };
  EXPECT_EQ(8, runFib(6));
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

