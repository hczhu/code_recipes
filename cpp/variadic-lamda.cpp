#include "../header.h"


TEST(Foo, Bar) {
  auto all = [&](...) { LOG(INFO) << "Called"; };
  all(1, 2, 3);
  all();
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

