#include "../header.h"


TEST(DeclareMess, Mess) {
  float a = 99.9;
  std::vector<int> b(size_t(a));
  static_assert(std::is_same_v<decltype(b), std::vector<int>>);

  std::vector<int> c{size_t(a)};
  static_assert(std::is_same_v<decltype(c), std::vector<int>>);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

