#include "../cpp/header.h"


struct A {
  int a;
  int b = 1;
  int c{-2};
  std::string haha = "haha";
};

TEST(Foo, Bar) {
  A a;
  EXPECT_EQ("haha", a.haha);

  const A& aa = A();
  EXPECT_EQ("haha", aa.haha);

  A aaa{
    .a = 10,
    .b = 20,
  };
  EXPECT_EQ(10, aaa.a);
  EXPECT_EQ(-2, aaa.c);
  EXPECT_EQ("haha", aaa.haha);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

