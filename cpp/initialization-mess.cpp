#include "../header.h"


TEST(InitializationMess, VectorMess) {
  B b;
  EXPECT_EQ(1, b.a.a);
  EXPECT_EQ(2, b.a.b);
  EXPECT_EQ(3, b.ints.size());
  EXPECT_EQ(3, b.shorts.size());
  std::vector<B> vb(2);
  for (const auto& b : vb) {
    EXPECT_EQ(1, b.a.a);
    EXPECT_EQ(2, b.a.b);
  }
  EXPECT_EQ(2, echo({ 10, 20 }).size());
  EXPECT_EQ(std::vector<int>({10, 20}), echo({ 10, 20 }));
  // std::vector has initializer_list ctor.
  {
    std::vector<int> v{10};
    EXPECT_EQ(1, v.size());
  }
  {
    std::vector<int> v({10});
    EXPECT_EQ(1, v.size());
  }
  {
    std::vector<int> v = {10, 20, 30};
    EXPECT_EQ(3, v.size());
  }
  {
    std::vector<int> v(10, 20);
    EXPECT_EQ(10, v.size());
  }
  // std::string has initializer_list<char> ctor.
  {
    std::string s{10, 'a'};
    EXPECT_EQ(2, s.length());
    EXPECT_EQ('a', s[1]);
  }
  {
    std::string s({10, 'a'});
    EXPECT_EQ(2, s.length());
    EXPECT_EQ('a', s[1]);
  }
  {
    std::string s = {10, 'a'};
    EXPECT_EQ(2, s.length());
    EXPECT_EQ('a', s[1]);
  }
  {
    A a{1, 2};
    EXPECT_EQ(1, a.a);
    EXPECT_EQ(2, a.b);
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

