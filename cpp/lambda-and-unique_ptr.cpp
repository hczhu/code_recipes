#include <header.h>


void foo(std::function<void()>&& f) {
  f();
}

TEST(Foo, Bar) {
  auto p = std::make_unique<int>(10);
  // Doesn't complile, because std::function requires the function object it
  // wraps to be CopyConstructible.
  // std::function<void()> f = [p = std::move(p)]() mutable { EXPECT_EQ(10, *p); };
  folly::Function<void()> f = [p = std::move(p)]() mutable {
    EXPECT_EQ(10, *p);
  };
  f();
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

