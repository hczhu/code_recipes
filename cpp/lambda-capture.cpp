#include "../header.h"


TEST(Foo, Bar) {
  Base a;
  auto f = [aa = std::move(a)]() mutable {
    std::cout << "Move lambda capture." << std::endl;
    aa.set(-100);
    aa.print();
  };
  std::cout << "---------------" << std::endl;
  auto f1 = [b = a] {
    std::cout << "Copy lambda capture." << std::endl;
    b.print();
  };
  std::cout << "---------------" << std::endl;
  f();
  f1();
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

