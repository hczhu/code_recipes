#include "../cpp/header.h"


#include <cassert>

int comp(const char *a, const char *b) {
  while (*a && *b) {
    auto la = tolower(*a++);
    auto lb = tolower(*b++);
    if (la != lb) {
      return (int)la - lb;
    }
  }
  return int(*a) - *b;
}

class CIString {
 public:
	CIString(std::string _s): s_(_s) {};
	CIString(const char* s): p_(s) {};
  const char* c_str() const {
    return p_ ? p_ : s_.c_str();
  }

 private:
	std::string s_;
  const char* p_ = nullptr;
};

bool operator<(const CIString &cs1, const CIString &cs) {
  return comp(cs1.c_str(), cs.c_str()) < 0;
}
bool operator>(const CIString &cs1, const CIString &cs) {
  return comp(cs1.c_str(), cs.c_str()) > 0;
}
bool operator<=(const CIString &cs1, const CIString &cs) {
  return comp(cs1.c_str(), cs.c_str()) <= 0;
}
bool operator>=(const CIString &cs1, const CIString &cs) {
  return comp(cs1.c_str(), cs.c_str()) >= 0;
}
bool operator==(const CIString &cs1, const CIString &cs) {
  return comp(cs1.c_str(), cs.c_str()) == 0;
}
bool operator!=(const CIString &cs1, const CIString &cs) {
  return comp(cs1.c_str(), cs.c_str()) != 0;
}


TEST(Foo, Bar) {
  CIString a("ab");
  CIString b("AB");
  EXPECT_TRUE(a == b);
  EXPECT_TRUE(a == "Ab");
  EXPECT_TRUE("aB" == a);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

