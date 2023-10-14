#include <header.h>


TEST(SharedPtrToUnique, Basic) {
  auto sptr = std::make_shared<int>(-100);
  PEEK(sptr.get());
  std::unique_ptr<int> uptr(new int(*sptr));
  PEEK(uptr.get());
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

