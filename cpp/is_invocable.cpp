#include "../header.h"


class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

struct A {}; struct B {}; struct C {};

struct Foo {
  int operator()(A const&) const { return 0; }
  int operator()(B const&) const { return 0; }
};

struct Bar {
  template <typename T, typename = std::enable_if_t<std::is_invocable_v<Foo, T>, void>>
  auto operator()(T const& v) const {
    return Foo{}(v);
  }
};

struct Bar2 {
  template <typename T>
  auto operator()(T const& v) const -> decltype(Foo{}(v)){
    return Foo{}(v);
  }
};

TEST_F(FooTest, Bar) {
  static_assert(std::is_invocable_v<Foo,A>, "");
  static_assert(std::is_invocable_v<Foo,B>, "");
  static_assert(!std::is_invocable_v<Foo,C>, "");

  static_assert(std::is_invocable_v<Bar,A>, "");
  static_assert(std::is_invocable_v<Bar,B>, "");
  static_assert(!std::is_invocable_v<Bar,C>, ""); 

  static_assert(std::is_invocable_v<Bar2,A>, "");
  static_assert(std::is_invocable_v<Bar2,B>, "");
  static_assert(!std::is_invocable_v<Bar2,C>, ""); 
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

