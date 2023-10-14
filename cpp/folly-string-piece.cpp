#include <header.h>


class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(FooTest, Bar) {
  std::vector<std::string> buf;
  const char *ptr = nullptr;
  {
    std::string haha("haha");
    folly::StringPiece sp(haha);
    buf.emplace_back(sp);
    LOG(INFO) << reinterpret_cast<uintptr_t>(sp.data()) << " v.s. "
              << reinterpret_cast<uintptr_t>(haha.data());
    EXPECT_EQ(sp.data(), haha.data());
    ptr = haha.data();
  }
  EXPECT_EQ(1, buf.size());
  LOG(INFO) << buf.front();
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

