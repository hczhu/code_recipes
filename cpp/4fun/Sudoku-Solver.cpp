#include <iostream>

#include <vector>
#include <cstring>
#include <iostream>

#define TWO(n) (1 << (n))
#define For9(r, c) for (int r = 0; r < 9; ++r) for (int c = 0; c < 9; ++c)
#define Cell(r, c) (r / 3 * 3 + c / 3)

class SudokuSolver {
 public:
  SudokuSolver() = default;
  bool solve_(std::vector<std::vector<char>>& board);
  bool solve(std::vector<std::vector<char>>& board) {
    init(board);
    return solve_(board);
  }

 private:
   void setDigit(int r, int c, int d) {
     rowBitMask_[r] ^= TWO(d);
     colBitMask_[c] ^= TWO(d);
     cellBitMask_[Cell(r, c)] ^= TWO(d);
   }

  void init(std::vector<std::vector<char>>& board) {
    memset(rowBitMask_, 0, sizeof(rowBitMask_));
    memset(colBitMask_, 0, sizeof(colBitMask_));
    memset(cellBitMask_, 0, sizeof(cellBitMask_));
    For9(r, c) {
      if (board[r][c] != '.') {
        setDigit(r, c, board[r][c] - '1');
      }
    }
  }
  uint32_t rowBitMask_[9];
  uint32_t colBitMask_[9];
  uint32_t cellBitMask_[9];
};

bool SudokuSolver::solve_(std::vector<std::vector<char>>& board) {
  int bestR = -1, bestC = -1, minFeasibleDigits = 10;


  For9(r, c) {
    if (board[r][c] == '.') {
      int numFeasibleBits = 9 - __builtin_popcount(rowBitMask_[r] | colBitMask_[c] | cellBitMask_[Cell(r, c)]);
      if (numFeasibleBits < minFeasibleDigits) {
        minFeasibleDigits = numFeasibleBits;
        bestR = r; bestC = c;
      }
    }
  }

  if (minFeasibleDigits > 9) {
    return true;
  }
  const int r = bestR;
  const int c = bestC;
  const auto usedBitsMask =
      rowBitMask_[r] | colBitMask_[c] | cellBitMask_[Cell(r, c)];
  for (int d = 0; d < 9; ++d) {
    if ((usedBitsMask & TWO(d)) == 0) {
      // std::cout << "set (" << r << ", " << c << ") to " << d << std::endl;
      setDigit(r, c, d);
      board[r][c] = d + '1';
      if (solve_(board)) {
        return true;
      }
      setDigit(r, c, d);
      board[r][c] = '.';
    }
  }
  return false;
}

int main() {
  std::vector<std::vector<char>> board = {
      { '5', '3', '.', '.', '7', '.', '.', '.', '.', },
      { '6', '.', '.', '1', '9', '5', '.', '.', '.', },
      { '.', '9', '8', '.', '.', '.', '.', '6', '.', },
      { '8', '.', '.', '.', '6', '.', '.', '.', '3', },
      { '4', '.', '.', '8', '.', '3', '.', '.', '1', },
      { '7', '.', '.', '.', '2', '.', '.', '.', '6', },
      { '.', '6', '.', '.', '.', '.', '2', '8', '.', },
      { '.', '.', '.', '4', '1', '9', '.', '.', '5', },
      { '.', '.', '.', '.', '8', '.', '.', '7', '9', },
  };

  SudokuSolver solver;
  solver.solve(board);


  for (const auto& row : board) {
    for (const auto cell : row) {
      std::cout << cell;
    }
    std::cout << std::endl;
  }
  return 0;
}
