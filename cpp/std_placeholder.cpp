#include <header.h>



template<class OutputType>
std::enable_if_t<std::is_placeholder<OutputType>::value == 0, void>
toMaybePlaceholder(std::string input, OutputType& output) {
  PEEK(input);
  output = input;
}

template<class OutputType>
std::enable_if_t<std::is_placeholder<OutputType>::value, void>
  toMaybePlaceholder(std::string, OutputType& output) {
}

void toOrIgnore(std::string, decltype(std::ignore)&) {
}


template <class OutputType>
// std::enable_if_t<!std::is_same<OutputType, decltype(std::ignore)>::value, void>
void toOrIgnore(std::string input, OutputType& output) {
  static_assert(std::is_same<OutputType, decltype(std::ignore)>::value == 0,
                "sholdn't be std::ignore");
  output = input;
}


TEST(Foo, Bar) {
  namespace Pl = std::placeholders;
  std::string output;
  toMaybePlaceholder("haha", output);
  EXPECT_EQ("haha", output);
  
  toMaybePlaceholder("", Pl::_1);
  EXPECT_EQ("haha", output);
  toMaybePlaceholder("", Pl::_2);
  EXPECT_EQ("haha", output);
  static_assert(std::is_placeholder<decltype(std::ignore)>::value == 0,
    "shouldn't be a placeholder.");

  toOrIgnore("", std::ignore);
  EXPECT_EQ("haha", output);
  toOrIgnore("", output);
  EXPECT_EQ("", output);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

