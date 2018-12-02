#include <string>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <fstream>

template<typename T>
void _displayType(T&& t);

TEST(Foo, Bar) {
  using namespace std;
  fstream myfile("/tmp/example.txt",
                 fstream::in | fstream::out | fstream::trunc);
  myfile << "a b c d";
  myfile.flush();
  std::string token;
  myfile.seekp(0);
  myfile >> token;
  EXPECT_EQ("a", token);
  myfile >> token;
  EXPECT_EQ("b", token);
  myfile >> token;
  EXPECT_EQ("c", token);
  myfile >> token;
  EXPECT_EQ("d", token);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
