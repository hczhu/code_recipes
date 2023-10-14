#include "../header.h"


void foo(std::chrono::milliseconds mil) {
  LOG(INFO) <<  mil.count() << " milliseconds.";
}

TEST(Foo, Bar) {
  foo(std::chrono::milliseconds(100));
  foo(std::chrono::seconds(100));
  foo(std::chrono::minutes(100));
  foo(std::chrono::hours(100));
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

