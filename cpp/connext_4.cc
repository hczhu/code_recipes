// This is a Connect-4 game solver. It does a brute-force search of the game
// tree.

#include "header.h"

// The game board is 7 columns by 6 rows.
// board[i + P] is column i, with board[i + P][P] the bottom cell in the column.
constexpr int P = 3;
// uint8_t board[7 + 2 * P][6 + 2 * P]; // padding to avoid bounds checking

constexpr uint8_t EMPTY = 0;
constexpr uint8_t RED = 1;
constexpr uint8_t YELLOW = 2;
constexpr char Winner[][7] = {"Draw", "Red", "Yellow"};
constexpr char Color[] = " RY";

#define SET(board, col, row, color) board[(col) + P][(row) + P] = (color)
#define CLEAR(board, col, row) board[(col) + P][(row) + P] = EMPTY
#define CTEST(board, col, row, color) (board[(col) + P][(row) + P] & (color))

// Count the number of pieces of the given color in a row starting from (col +
// dcol, row + drow). Use built-in bitwise operations to speed up the check.
#define COUNT(board, col, row, dcol, drow, color)                            \
  __builtin_popcount(                                                        \
      CTEST(board, (col) + (dcol), (row) + (drow), (color)) ^                \
      (CTEST(board, (col) + 2 * (dcol), (row) + 2 * (drow), (color)) << 2) ^ \
      (CTEST(board, (col) + 3 * (dcol), (row) + 3 * (drow), (color)) << 4))

bool isWinningMove(uint8_t board[][6 + 2 * P],
                   int col,
                   int row,
                   uint8_t color) {
  return COUNT(board, col, row, 1, 0, color) +
                 COUNT(board, col, row, -1, 0, color) >=
             3 ||
         COUNT(board, col, row, 0, 1, color) +
                 COUNT(board, col, row, 0, -1, color) >=
             3 ||
         COUNT(board, col, row, 1, 1, color) +
                 COUNT(board, col, row, -1, -1, color) >=
             3 ||
         COUNT(board, col, row, 1, -1, color) +
                 COUNT(board, col, row, -1, 1, color) >=
             3;
}

constexpr int HEIGHTS_SHIFT = 42;
constexpr int COLOR_SHIFT = 64 - HEIGHTS_SHIFT;

#define HEIGHT_BITS(key) (key >> HEIGHTS_SHIFT)
#define COLOR_BITS(key) ((key << COLOR_SHIFT) >> COLOR_SHIFT)

// heights uses 3 bits per column to represent the height of each column.
// colorBits uses 42 (7 columns * 6 rows) bits to represent the board state.
uint64_t setColor(uint8_t board[][6 + 2 * P],
                  uint64_t key,
                  int col,
                  uint8_t color) {
  auto heights = HEIGHT_BITS(key);
  int row = (heights >> (3 * col)) & 0x7;
  SET(board, col, row, color);
  return ((heights + (1ULL << (3 * col))) << HEIGHTS_SHIFT) ^
         (COLOR_BITS(key) ^ (uint64_t(color == YELLOW) << (col * 6 + row)));
}

void printBoard(uint8_t board[][6 + 2 * P], uint64_t key) {
  auto hieghts = HEIGHT_BITS(key);
  for (int col = 0; col < 7; col++, hieghts >>= 3) {
    std::cout << col << ": ";
    for (int row = 0; row < (hieghts & 0x7); row++) {
      std::cout << Color[board[col + P][row + P]];
    }
    std::cout << std::endl;
  }
}

// The results are
// EMPTY: No winner at the end of the game (draw).
// RED: Red wins.
// YELLOW: Yellow wins.
std::unordered_map<uint64_t, uint8_t> memo;

uint8_t treeSearch(uint8_t board[][6 + 2 * P],
                   const uint64_t key,
                   uint8_t color,
                   int rounds) {
  if (rounds == 42) {
    return EMPTY;
  }
  auto itr = memo.find(key);
  if (itr != memo.end()) {
    return itr->second;
  }
  auto heights = key >> HEIGHTS_SHIFT;
  for (int col = 0; col < 7; col++, heights >>= 3) {
    if ((heights & 0x7) < 6 &&
        isWinningMove(board, col, heights & 0x7, color)) {
      return color;
    }
  }
  heights = key >> HEIGHTS_SHIFT;
  auto res = color ^ 3;
  for (int col = 0; col < 7; col++, heights >>= 3) {
    auto row = heights & 0x7;
    if (row >= 6) {
      continue;
    }
    auto newKey = setColor(board, key, col, color);
    auto winner = treeSearch(board, newKey, color ^ 3, rounds + 1);
    CLEAR(board, col, row);
    if (winner == color) {
      res = color;
      break;
    }
    if (winner == EMPTY) {
      res = EMPTY;
    }
  }
  memo[key] = res;
  if (memo.size() % 100000 == 0) {
    LOG(INFO) << "Memo size: " << memo.size();
  }
  /*
  std::cout << "==== " << Winner[res] << " wins at rounds " << rounds
            << " for color " << Color[color] << " heights " << std::oct
            << HEIGHT_BITS(key) << " colors "
            << std::bitset<64>(COLOR_BITS(key)) << " ====" << std::endl;
  printBoard(board, key);
  */
  return res;
}

TEST(Connect4, Basics) {
  uint8_t board[7 + 2 * P][6 + 2 * P];
  memset(board, 0, sizeof(board));
  SET(board, 3, 0, RED);
  EXPECT_FALSE(isWinningMove(board, 3, 1, RED));
  SET(board, 3, 1, RED);
  EXPECT_FALSE(isWinningMove(board, 3, 2, RED));
  SET(board, 3, 2, RED);
  EXPECT_TRUE(isWinningMove(board, 3, 3, RED));
  EXPECT_FALSE(isWinningMove(board, 2, 2, RED));
  CLEAR(board, 3, 2);
  EXPECT_FALSE(isWinningMove(board, 3, 2, RED));
  SET(board, 0, 0, YELLOW);
  SET(board, 1, 1, YELLOW);
  SET(board, 2, 2, YELLOW);
  EXPECT_TRUE(isWinningMove(board, 3, 3, YELLOW));
  EXPECT_FALSE(isWinningMove(board, 3, 3, RED));

  memset(board, 0, sizeof(board));
  uint64_t key = 0;
  key = setColor(board, key, 3, RED);
  key = setColor(board, key, 3, YELLOW);
  key = setColor(board, key, 2, RED);
  key = setColor(board, key, 2, YELLOW);
  printBoard(board, key);
  uint64_t key2 = 0;
  memset(board, 0, sizeof(board));
  key2 = setColor(board, key2, 2, RED);
  key2 = setColor(board, key2, 2, YELLOW);
  key2 = setColor(board, key2, 3, RED);
  key2 = setColor(board, key2, 3, YELLOW);
  printBoard(board, key2);
  EXPECT_EQ(key, key2);
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  auto res = RUN_ALL_TESTS();
  if (res != 0) {
    return res;
  }
  uint8_t board[7 + 2 * P][6 + 2 * P];
  memset(board, 0, sizeof(board));
  auto winner = treeSearch(board, 0, RED, 0);
  LOG(INFO) << Winner[winner] << " will win. Memo size: " << memo.size();
  return res;
}
