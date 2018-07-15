#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <iostream>
#include <optional>
#include <stack>
#include <list>
#include <vector>
using namespace std;
using std::tuple;
#include "generator/io.inc.h"
#define SLOW_BASE

class Grid : public vector<uint8_t> {
public:
  Grid() : vector(DIM * DIM, 0) {}
  uint8_t &operator()(int row, int col) {
    // hhh
    return (*this)[row * DIM + col];
  }
  void show() {
    bool flag = true;
    for (int row = 0; row < DIM; ++row) {
      for (int col = 0; col < DIM; ++col) {
        auto value = (int) (*this)(row, col);
        cout << value;
      }
      cout << endl;
    }
    cout << endl;
  }
};

struct Engine {
  std::list<Grid> candidate;
  void push(Grid &&g) { candidate.push_back(std::move(g)); }
  std::optional<Grid> pop() {
    if (candidate.empty()) {
      return std::nullopt;
    }
    Grid g = std::move(candidate.back());
    candidate.pop_back();
    return g;
  }
};

int numberOfSetBits(unsigned i) { return __builtin_popcount(i & ~1); }
int leastSignificantBit(unsigned i) { return ffs(i & ~1) - 1; }

void kernel(Grid grid, Engine &eng) {
  unsigned rowflag[9] = {};
  unsigned colflag[9] = {};
  unsigned blockflag[9] = {};

  auto get_cell_flag = [&](int row, int col) {
    int block = row / 3 * 3 + col / 3;
    return rowflag[row] | colflag[col] | blockflag[block];
  };

  auto set_cell = [&](int row, int col, uint8_t value) {
    int block = row / 3 * 3 + col / 3;
    unsigned bit = 1 << value;

    assert(0 == (rowflag[row] & bit));
    assert(0 == (colflag[col] & bit));
    assert(0 == (blockflag[block] & bit));
    grid[row*DIM + col]  = value;
    rowflag[row] |= bit;
    colflag[col] |= bit;
    blockflag[block] |= bit;
  };

  for (int row = 0; row < DIM; ++row) {
    for (int col = 0; col < DIM; ++col) {
      int block = row / 3 * 3 + col / 3;
      auto ele = grid(row, col);
      if (ele) {
        set_cell(row, col, ele);
        int i = 1 + 1;
      }
    }
  }

  bool advanced;
  int max_known = -1;
  int max_row = -1, max_col = -1;
  // forward until has to branch
  do {
    max_known = -1;
    max_row = -1, max_col = -1;
    advanced = false;
    for (int row = 0; row < DIM; ++row) {
      for (int col = 0; col < DIM; ++col) {
        auto cell = grid(row, col);
        if (cell != 0) {
          continue;
        }
        auto cellflag = get_cell_flag(row, col);
        int known = numberOfSetBits(cellflag);
        #ifndef SLOW_BASE
        if (known == 8) {
          auto cellflag = get_cell_flag(row, col);
          auto cellvalue = leastSignificantBit(~cellflag);
          set_cell(row, col, cellvalue);
          advanced = true;
          continue;
        }
        #endif // SLOW_BASE
        if(known == 9){
          return;
        }
        if (!advanced
          #ifndef SLOW_BASE
            && max_known < known
          #endif // SLOW_BASE
            ) {
          max_row = row;
          max_col = col;
          max_known = known;
        }
      }
    }
  } while (advanced);

  if (max_known == -1) {
    static int count = 100;
    count--;
    grid.show();
    if(count < 0){
      exit(0);
    }
    return;
  }
  // done
  int trial = max_known;
  auto cellflag = get_cell_flag(max_row, max_col);
  for (int v = 1; v <= DIM; ++v) {
    unsigned bit = 1 << v;
    if ((bit & cellflag) == 0) {
      Grid clone = grid;
      clone(max_row, max_col) = v;
      eng.candidate.push_back(std::move(clone));
      ++trial;
    }
  }
  assert(trial == 9);
}

std::optional<Grid> solve(Engine &eng) {
  while (true) {
    auto grid_opt = eng.pop();
    if (!grid_opt) {
      return std::nullopt;
    }
    auto grid = grid_opt.value();

    static int id = 0;
    kernel(std::move(grid), eng);
  }
}

int main() {
  freopen("/home/mike/workspace/parallel_lab/project-sudoku/data/data.txt", "r", stdin);
  Engine eng;
  Grid grid;
  read_grid(grid.data());
  eng.candidate.push_back(std::move(grid));

  solve(eng);
}
