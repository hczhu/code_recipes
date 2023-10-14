#include <header.h>


class Foo {
  struct PrivateTag {};

 public:
  Foo(int a, PrivateTag) : a_(a) {}

  void bar(PrivateTag) {}

  static void sbar(PrivateTag) {}

 private:
  const int a_;
};

template<typename T>
T extract(void(Foo::*)(T)) {
  return T();
}

template<typename T>
T extractByRef(void(&funRef)(T)) {
  funRef(T());
  return T();
}


TEST(Foo, Bar) {
  // Foo foo0(-1, extract(&Foo::Foo));

  Foo foo(-1, extract(&Foo::bar));
  Foo foo1(-1, extractByRef(Foo::sbar));
  auto p = &Foo::bar;
  // Doesn't compile
  // auto& pref = &Foo::bar;
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

