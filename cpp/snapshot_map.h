
#include "../cpp/header.h"

template <typename K, typename V>
class SnapshotMap {
 private:
  using Map = std::unordered_map<K, V>;

 public:
  SnapshotMap() = default;
  // https://en.cppreference.com/w/cpp/memory/shared_ptr/atomic2
};

class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(FooTest, Bar) {}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}
