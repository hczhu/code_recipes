#include "../header.h"


class LazyArray {
public:
  using MapFn = std::function<int(int)>;
  LazyArray() = default;
  LazyArray(std::vector<int> values) : values_(std::move(values)) {}

  LazyArray map(MapFn f) {
    LazyArray child;
    child.fn_ = std::move(f);
    child.parent_ = this;
    return child;
  }

  // std::string::npos == maximum size_t
  size_t indexOf(int target) {
    std::vector<const MapFn *> fns;
    auto p = this;
    while (p->parent_ != nullptr) {
      fns.push_back(&(p->fn_));
      p = p->parent_;
    }
    const auto &values = p->values_;
    std::reverse(fns.begin(), fns.end());
    for (size_t i = 0; i < values.size(); ++i) {
      auto v = values[i];
      for (auto &f : fns) {
        v = (*f)(v);
      }
      if (v == target) {
        return i;
      }
    }
    return std::string::npos;
  }

private:
  LazyArray *parent_ = nullptr;
  const std::vector<int> values_;
  MapFn fn_;
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
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

