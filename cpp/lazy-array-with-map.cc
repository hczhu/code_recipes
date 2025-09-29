#include "../cpp/header.h"


class LazyArray {
 public:
  using MapFn = std::function<int(int)>;

 private:
  std::shared_ptr<std::vector<int> > values_;
  MapFn fn_;

  LazyArray(std::shared_ptr<std::vector<int>> values, MapFn f)
      : values_(values),
        fn_(std::move(f)) {}

 public:
  LazyArray(std::vector<int> values) : values_(std::make_shared<std::vector<int> >(move(values))) {}

  LazyArray map(MapFn f) {
    if (fn_) {
      return LazyArray(
        values_,
        [prev_f=fn_, ff=std::move(f)](int v) { 
          return ff(prev_f(v));
        }
      );
    }
    return LazyArray(values_, std::move(f));
  }

  // std::string::npos == maximum size_t
  size_t indexOf(int needle) {
    auto needle_itr =
        std::find_if(values_->begin(), values_->end(), [this, needle](int v) {
          return (this->fn_ ? this->fn_(v) : v) == needle;
        });
    if (needle_itr == values_->end()) {
      return std::string::npos;
    }
    return std::distance(values_->begin(), needle_itr);
  }
};

TEST(LazyArrayTest, MapTest) {
  LazyArray a{{10, 20, 30, 40, 50}};
  std::function<int(int)> timesTwo = [](int i) { return i * 2; };

  EXPECT_EQ(a.map(timesTwo).indexOf(40), 1);
  EXPECT_EQ(a.map(timesTwo).indexOf(40), 1);
  EXPECT_EQ(a.map(timesTwo).map(timesTwo).indexOf(80), 1);
  auto b = a.map(timesTwo);
  auto c = b.map(timesTwo);
  EXPECT_EQ(b.indexOf(80), 3);

  EXPECT_EQ(c.indexOf(80), 1);

  EXPECT_EQ(c.indexOf(80), 1);

  EXPECT_EQ(a.indexOf(50), 4);

  EXPECT_EQ(a.map(timesTwo).indexOf(101), std::string::npos);

  int called = 0;
  std::function<int(int)> timesTwoForTest = [&](int i) {
    ++called;
    return i * 2;
  };

  auto d = a.map(timesTwoForTest).map(timesTwoForTest);

  EXPECT_EQ(called, 0);
  EXPECT_EQ(d.indexOf(40), 0);
  EXPECT_EQ(called, 2);

  {
    LazyArray a{{20, 10, 20, 30, 40, 50}};
    EXPECT_EQ(a.map(timesTwo).map(timesTwo).indexOf(80), 0);
  }
  auto e = a.map([](int v) {return v;});
  EXPECT_EQ(a.map(timesTwo).map(timesTwo).indexOf(80), 1);
  {
    auto b = a.map(timesTwo);
    auto c = b.map(timesTwo);
    auto d = c.map(timesTwo);
    e = d.map(timesTwo);
  }
  EXPECT_EQ(e.indexOf(320), 1);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

