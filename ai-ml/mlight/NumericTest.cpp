#include "../header.h"


TEST(Foo, Bar) {
  for (double x = 40; x < 100; x += 1) {
    LOG(INFO) << "exp(" << x << ")=" << exp(x) << " exp(" << -x
              << ")=" << exp(-x) << " sigmod(x) = " << (1 / (1 + exp(-x)))
              << " sigmod(-x) = " << (1 / (1 + exp(x)));
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

