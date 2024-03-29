#include "../cpp/header.h"


/*
 * An implementation of the algorithm in the paper
 * "A new flexible algorithm for the longest common subsequence problem"
 * The time complexity is O(min(pm,p(n-p))) where m and n (m<=n) are lengths of
 * two strings , and p is the length of LCS.
 *
 */

size_t longestCommonSubsequence(const std::string &str1Input,
                                const std::string &str2Input) {
  auto str1 = reinterpret_cast<const unsigned char *>(str1Input.data());
  auto str2 = reinterpret_cast<const unsigned char *>(str2Input.data());
  size_t n1 = str1Input.size();
  size_t n2 = str2Input.size();

  if (n1 > n2) {
    std::swap(str1, str2);
    std::swap(n1, n2);
  }

  // Now, str1.size() <= str2.size()
  const size_t s = 1 + std::max(*std::max_element(str1, str1 + n1),
                                *std::max_element(str2, str2 + n2));

  auto getNextPos = [](size_t n, size_t s, const unsigned char *str) {
    // nextPos[a][b] points to the smallest 'i' such that i >= a && str[i] == b
    std::vector<std::vector<int>> nextPos(n + 2, std::vector<int>(s));
    std::fill(nextPos[n + 1].begin(), nextPos[n + 1].end(), n);
    nextPos[n] = nextPos[n + 1];

    for (int i = n - 1; i >= 0; i--) {
      nextPos[i] = nextPos[i + 1];
      nextPos[i][str[i]] = i;
    }

    return nextPos;
  };

  const auto nextPos1 = getNextPos(n1, s, str1);
  const auto nextPos2 = getNextPos(n2, s, str2);

  std::vector<int> rowMatch(n1 + 2, n2), colMatch(n1 + 2, n1);
  colMatch[0] = rowMatch[0] = -1;

  int low = 1;
  for (int i = 0; i < n1; i++) {
    int c = colMatch[low] == i ? (nextPos2[rowMatch[low++] + 1][str1[i]])
                               : nextPos2[i][str1[i]];
    for (int k = low; c < n2; k++) {
      const auto tmp = rowMatch[k];
      if (tmp >= c) {
        rowMatch[k] = c;
        c = nextPos2[tmp + 1][str1[i]];
      }
    }
    int r = rowMatch[low] == i ? (nextPos1[colMatch[low++] + 1][str2[i]])
                               : nextPos1[i + 1][str2[i]];
    for (int k = low; r < n1; k++) {
      const auto tmp = colMatch[k];
      if (tmp >= r) {
        colMatch[k] = r;
        r = nextPos1[tmp + 1][str2[i]];
      }
    }
  }
  while (rowMatch[low] < n2) {
    low++;
  }
  return low - 1;
}

TEST(LCS_test, Basic) {
  EXPECT_EQ(longestCommonSubsequence("a1b13c234123d", "9a99b999cd999"), 4);

  auto testRandomStrings = [](size_t n1, size_t n2) {
    std::mt19937 rnd(std::time(nullptr));
    std::uniform_int_distribution<size_t> dist(1, 6);

    auto getStr = [&](size_t n) {
      std::string s(n, '0');
      for (size_t i = 0; i < n; ++i) {
        s[i] = dist(rnd);
      }
      return s;
    };

    const auto str1 = getStr(n1);
    const auto str2 = getStr(n2);

    std::vector<int> m((n1 + 1) * (n2 + 1));
#define D(i, j) m[(i) * (n2 + 1) + (j)]
    for (int i = 1; i <= n1; ++i) {
      for (int j = 1; j <= n2; ++j) {
        D(i, j) = str1[i - 1] == str2[j - 1]
                      ? (1 + D(i - 1, j - 1))
                      : std::max(D(i - 1, j), D(i, j - 1));
      }
    }

    LOG(INFO) << "LCS for two random strings: " << D(n1, n2);
    EXPECT_EQ(D(n1, n2), longestCommonSubsequence(str1, str2));
#undef D
  };

  testRandomStrings(10, 100);
  testRandomStrings(50, 100);
  testRandomStrings(100, 60);
  testRandomStrings(500, 100);
  testRandomStrings(300, 200);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  return RUN_ALL_TESTS();
}

