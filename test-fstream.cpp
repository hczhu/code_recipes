#include <string>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <gtest/gtest.h>
#include <fstream>

template<typename T>
void _displayType(T&& t);

TEST(Foo, Bar) {
  using namespace std;
  {
    ofstream("/tmp/example.txt");
  }
  fstream myfile;
  myfile.open("/tmp/example.txt", ios::in | ios::out);
  myfile << "a b c d";
  myfile.flush();
  std::string token;
  myfile.seekp(0);
  myfile >> token;
  EXPECT_EQ("a", token);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
