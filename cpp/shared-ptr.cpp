#include "../header.h"


struct Foo : public std::enable_shared_from_this<Foo> { };

class A {

};

class B : public A {

};

void foo(std::shared_ptr<A>) {

}

TEST(Foo, Bar) {
  foo(std::make_shared<B>());
  {
    auto p1 = std::make_shared<Foo>();
    EXPECT_EQ(1, p1.use_count());
    auto p2 = p1;
    EXPECT_EQ(2, p1.use_count());
    
    auto rawPtr = p1.get();
    std::shared_ptr<Foo> p3 = rawPtr->shared_from_this();
    EXPECT_EQ(3, p1.use_count());
    EXPECT_EQ(3, p3.use_count());
  }
  {
    auto p1 = std::make_shared<std::string>();
    EXPECT_EQ(1, p1.use_count());
    auto p2 = p1;
    EXPECT_EQ(2, p1.use_count());
    /* Lead to a runtime error:
    std::shared_ptr<std::string> p3(p1.get());
    EXPECT_EQ(2, p1.use_count());
    EXPECT_EQ(1, p3.use_count());
    */
  }
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

