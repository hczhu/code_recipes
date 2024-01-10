#include "../cpp/header.h"

struct Data {
  std::string a;
  int b;
  struct Inner {
    bool c;
    bool d;
  };
  Inner inner;
};

template<class T, typename F>
auto get_lambda(F T::*field) {
  return [=](const T& t) -> F{
    return t.*field;
  };
}

int main(int argc, char* argv[]) {
  auto ptr = &Data::Inner::c;
  Data d;
  d.inner.c = true;
  d.b = -1;
  PEEK(d.inner.*ptr);

  auto lm = get_lambda(&Data::b);

  EXPECT_EQ(-1, lm(d));

  // _DisplayType<decltype(&Data::Inner::c)> _a;
  return 0;
}

