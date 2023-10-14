#include "../header.h"


TEST(ForwardIterator, Basic) {
  using Itr = std::iterator<std::input_iterator_tag, int>;
  std::vector<int> values = {
    1, 2, 3, 4, 5
  };
  Itr bitr(std::cbegin(values));
  Itr eitr = values.cend();
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

