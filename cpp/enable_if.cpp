#include "../cpp/header.h"


template<typename T>
struct is_bool_or_int : std::false_type {};

template<>
struct is_bool_or_int<bool> : std::true_type {};

template<>
struct is_bool_or_int<int> : std::true_type {};

template<typename T>
std::enable_if_t<is_bool_or_int<T>::value, bool> foo(T) {
  return true;
}

template<typename T>
std::enable_if_t<!is_bool_or_int<T>::value, bool> foo(T) {
  return false;
}

template<typename T, bool R = is_bool_or_int<T>::value>
bool bar(T) {
  return R;
}

TEST(Foo, Bar) {
  EXPECT_TRUE(foo(1));
  EXPECT_TRUE(foo(false));
  EXPECT_FALSE(foo(.33));
  EXPECT_FALSE(foo("adad"));

  EXPECT_TRUE(bar(1));
  EXPECT_TRUE(bar(false));
  EXPECT_FALSE(bar(.33));
  EXPECT_FALSE(bar("adad"));
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

