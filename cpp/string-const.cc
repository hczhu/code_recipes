#include "../header.h"


// const folly::StringPiece
inline folly::StringPiece constexpr spVariable() {
  return "const-sp";
}

// const foly::FixedString.
inline auto constexpr prefix() {
  return  folly::makeFixedString("prefix");
}

inline auto constexpr constFS() {
  return prefix() + folly::makeFixedString(".suffix");
}

// const string
static auto const gV = std::string_literals::operator""s("const-string", 11);

using namespace std::string_literals;
static auto const variable = "const-string"s;

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  // _displayType(derived());
  PEEK(spVariable());
  PEEK(prefix());
  PEEK(constFS());
  PEEK(gV);
  PEEK(variable);
  return 0;
}
