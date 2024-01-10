
#include "../cpp/header.h"

class Dijkstra {
 public:
};

class DijkstraTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(DijkstraTest, Bar) {
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

