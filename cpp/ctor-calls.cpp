#include <header.h>


TEST(Foo, Bar) {
  auto bb = Base();
  EXPECT_EQ(1, Base::CtorCnt);
  EXPECT_EQ(0, Base::MoveCnt);
  EXPECT_EQ(0, Base::CopyCnt);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

