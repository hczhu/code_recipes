#include "../header.h"


class CaptureByValueTrap : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(CaptureByValueTrap, Basic) {
  int v = 0;
  auto f = [=] ()mutable {
    return ++v;      
  };

  EXPECT_EQ(1, f());
  EXPECT_EQ(2, f());
  EXPECT_EQ(3, f());

  EXPECT_EQ(0, v);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

