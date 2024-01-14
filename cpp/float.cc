
#include "../cpp/header.h"

class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(FooTest, Bar) {
  auto print = [](const std::string& name, auto value) {
    std::cout << name << " : " << std::endl
              << std::fixed << " value = " << value << std::endl
              << std::scientific << " value = " << value << std::endl
              << std::hexfloat << " value = " << value << std::endl
              << "  max = " << std::numeric_limits<decltype(value)>::max()
              << std::endl
              << "  max / 2 = "
              << std::numeric_limits<decltype(value)>::max() / 2 << std::endl
              << "  min = " << std::numeric_limits<decltype(value)>::min() / 2
              << std::endl;
  };
  float v = 5;
  for (int i = 0; i < 30; ++i) {
    v /= 2;
  }
  print("float", v);
  print("double", double(v));
  print("long double", (long double)(v));
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
