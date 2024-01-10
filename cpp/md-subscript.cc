
#include "../cpp/header.h"

REQUIURE_CPP_STD(202100L);

template <typename V>
class TriangularMatrix {
 public:
  // Only has [i, j] with i <= j or i < j depending on hasDiagonal.
  TriangularMatrix(size_t n, bool hasDiagonal = true)
      : n_(n), hasDiagonal_(hasDiagonal) {
    auto vn = (n * n + n) / 2;
    if (!hasDiagonal_) {
      vn -= n;
    }
    values_.resize(vn);
  }

  int& operator[](size_t i, size_t j) & {
    checkSubscripts(i, j);
    return values_[getSubscript(i, j)];
  }

  const int& operator()(size_t i, size_t j) const {
    checkSubscripts(i, j);
    return values_[getSubscript(i, j)];
  }

 private:
  const size_t n_;
  const bool hasDiagonal_;
  std::vector<int> values_;

  inline size_t getSubscript(size_t i, size_t j) const {
    // hasDiagnoal=true: (n - 0) + (n - 1) +  ... + (n - i)
    //                    + (j - i)
    // hasDiagnoal=false: - i - 1
    return i * (2 * n_ - i) / 2 + (j - i) - (hasDiagonal_ ? 0 : i + 1);
  }

  void checkSubscripts(size_t i, size_t j) const {
    if (i >= n_ || j >= n_) {
      throw std::out_of_range("Subscripts out of range: " + std::to_string(i) + " or " + std::to_string(j) + " >= " + std::to_string(n_));
    }
    if (i > j) {
      throw std::out_of_range("subscript out of range: " + std::to_string(i) +
                              " > " + std::to_string(j));
    }
    if (!hasDiagonal_ && i == j) {
      throw std::out_of_range("subscript out of range: " + std::to_string(i) +
                              " == " + std::to_string(j));
    }
  }
};

class FooTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(FooTest, SimpleTest) {
  using TM = TriangularMatrix<int>;
  TM tm(7);
  EXPECT_THROW(tm(7, 0), std::out_of_range);
  EXPECT_THROW(tm(3, 2), std::out_of_range);
  for (size_t i = 0; i < 6; ++i) {
    tm[i, i] = i;
    tm[i, i + 1] = i + 1;
  }
  tm[6, 6] = 6;
  EXPECT_EQ(tm(0, 0), 0);
  EXPECT_EQ(tm(5, 5), 5);
  EXPECT_EQ(tm(5, 6), 6);
  EXPECT_EQ(tm(6, 6), 6);
  tm[5, 5] = 7;
  tm[5, 6] = -1;
  tm[6, 6] = -2;
  EXPECT_EQ(tm(5, 5), 7);
  EXPECT_EQ(tm(5, 6), -1);
  EXPECT_EQ(tm(6, 6), -2);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  PEEK(__cplusplus);
  return RUN_ALL_TESTS();
}
