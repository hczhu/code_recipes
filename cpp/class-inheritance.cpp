#include "../header.h"


TEST(Foo, Bar) {
  B b(-1);
  EXPECT_EQ(-1, b.getA());
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

