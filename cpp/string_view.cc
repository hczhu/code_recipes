#include <header.h>


TEST(StringView, Basic) {
  std::string s = "abcdefg"; 
  std::string_view ss = s;
  std::string_view sv(s.begin() + 2, s.begin() + 4);
  EXPECT_EQ(sv.size(), 2);
  EXPECT_EQ(std::distance(ss.begin(), sv.begin()), 2);
  sv.remove_prefix(1);
  EXPECT_EQ(std::distance(ss.begin(), sv.begin()), 3);

  std::string cs(sv);
  EXPECT_EQ(cs, "d");

  EXPECT_EQ(std::string_view("abcdefg"), ss);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

