#include <header.h>



TEST(Foo, Bar) {
  struct Container {
    Container() = default;
    Container(const std::string c) : a(c) { }
    const std::string a = "aaa";
    Base b;
    const Base d;
  };

  Container a("haha");
  Container b(std::move(a));
  EXPECT_EQ(1, Base::MoveCnt);
  EXPECT_EQ(1, Base::CopyCnt);
  EXPECT_EQ("haha", a.a);
  EXPECT_EQ("haha", b.a);

  auto c = std::move(b);
  EXPECT_EQ(2, Base::MoveCnt);
  EXPECT_EQ(2, Base::CopyCnt);
  EXPECT_EQ("haha", c.a);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

