
#include "../cpp/header.h"

void bar(std::function<void()>&& fn) {

}

TEST(Foo, Bar) {
  std::mutex mx;
  std::unique_lock<std::mutex> lg(mx);
  { auto lm = [_lg = move(lg)]{}; }
  EXPECT_TRUE(mx.try_lock());
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

