#include "../cpp/header.h"


TEST(Foo, Bar) {
  std::vector<int> v{
    std::istream_iterator<int>(std::cin),
    std::istream_iterator<int>()
  };
  PEEK(v.size());
  PEEK(v[0]++);
  PEEK(v[0]);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

