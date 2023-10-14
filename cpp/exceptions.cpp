#include "../header.h"


class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};


class NoneSenseException : public std::exception { };

TEST_F(FooTest, Bar) {
  auto f = [] {
    try {
      throw NoneSenseException();
    } catch (std::exception& ex) {
      throw;
    }
  };
  EXPECT_THROW(f(), NoneSenseException);

  auto ff = [] {
    try {
      throw NoneSenseException();
    } catch (std::exception& ex) {
      // 'ex' gets sliced.
      throw ex;
    }
  };
  EXPECT_THROW(f(), std::exception);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

