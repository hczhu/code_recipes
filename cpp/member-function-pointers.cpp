#include "../header.h"


TEST(MemberPointerTest, Basic) {
  EXPECT_EQ(&B::foo, &D::foo);
  EXPECT_NE(&B::bar, &D::bar);
  EXPECT_EQ(&B::den, &D::den);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

