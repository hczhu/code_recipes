#include "../cpp/header.h"
#include "dummy-classes.h"

class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

template<typename F>
void callF(F&& f) {
  static Base b;
  f();
}

template<typename F>
void createStatic() {
  static Base b;
}

TEST_F(FooTest, Bar) {
  Base a;
  {
    Base d;
    static Base c;
  }
  EXPECT_EQ(2, Base::LiveCnt);
  for (bool once = false; once;) {
    for (static Base d; once; once = false) {
    }
  }
  EXPECT_EQ(2, Base::LiveCnt);
  {
    auto logIt = [] {
      for (bool once = true; once;) {
        for (static Base d; once; once = false) {
        }
      }
    };
    std::vector<std::thread> threads;
    for (int t = 0; t < 10; ++t) {
      threads.emplace_back([logIt] {
        for (int i = 0; i < 100; ++i) {
          logIt();
          EXPECT_EQ(3, Base::LiveCnt);
          EXPECT_EQ(1, Base::DtorCnt);
        }
      });
    }
    for (auto &thr : threads) {
      thr.join();
    }
    EXPECT_EQ(3, Base::LiveCnt);
    EXPECT_EQ(1, Base::DtorCnt);
  }

  {
    std::vector<std::thread> threads;
    for (int t = 0; t < 10; ++t) {
      auto logIt = [t] {
        for (bool once = true; once;) {
          for (static Base d; once; once = false) {
            LOG(INFO) << "thread #" << t;
          }
        }
      };

      threads.emplace_back([logIt] {
        for (int i = 0; i < 100; ++i) {
          callF(logIt);
          EXPECT_EQ(5, Base::LiveCnt);
        }
      });
    }
    for (auto &thr : threads) {
      thr.join();
    }
    EXPECT_EQ(5, Base::LiveCnt);
  }

  createStatic<int>();
  EXPECT_EQ(6, Base::LiveCnt);
  createStatic<int>();
  EXPECT_EQ(6, Base::LiveCnt);

  createStatic<double>();
  EXPECT_EQ(7, Base::LiveCnt);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

