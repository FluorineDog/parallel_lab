#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <deque>
#include <iostream>
#include <list>
#include <optional>
#include <stack>
#include <vector>
#include "generator/io.inc.h"
using namespace std;
using namespace std::chrono;
using std::tuple;./
//#define SLOW_BASE

class Grid : public vector<uint8_t> {
 public:
  Grid() : vector(DIM * DIM, 0) {}
  uint8_t &operator()(int row, int col) {
    // hhh
    return (*this)[row * DIM + col];
  }
  void show() {
    //    constexpr auto str = "_0123456789ABCDEF";
    //    bool flag = true;
    //    for (int row = 0; row < DIM; ++row) {
    //      for (int col = 0; col < DIM; ++col) {
    //        auto value = str[(*this)(row, col)];
    //        cout << value << " ";
    //      }
    //      cout << "$" << endl;
    //    }
    //    cout << endl;
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
  unsigned rowflag[DIM] = {};
  unsigned colflag[DIM] = {};
  unsigned blockflag[DIM] = {};

  auto get_cell_flag = [&](int row, int col) {
    int block = row / ORDER * ORDER + col / ORDER;
    return rowflag[row] | colflag[col] | blockflag[block];
  };

  auto set_cell = [&](int row, int col, uint8_t value) {
    int block = row / ORDER * ORDER + col / ORDER;
    unsigned bit = 1 << value;

    assert(0 == (rowflag[row] & bit));
    assert(0 == (colflag[col] & bit));
    assert(0 == (blockflag[block] & bit));
    grid[row * DIM + col] = value;
    rowflag[row] |= bit;
    colflag[col] |= bit;
    blockflag[block] |= bit;
  };

  for (int row = 0; row < DIM; ++row) {
    for (int col = 0; col < DIM; ++col) {
      int block = row / ORDER * ORDER + col / ORDER;
      auto ele = grid(row, col);
      if (ele) {
        set_cell(row, col, ele);
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
        if (known == DIM - 1) {
          auto cellflag = get_cell_flag(row, col);
          auto cellvalue = leastSignificantBit(~cellflag);
          set_cell(row, col, cellvalue);
          advanced = true;
          continue;
        }
#endif  // SLOW_BASE
        if (known == DIM) {
          return;
        }
        if (!advanced
            //        #ifndef SLOW_BASE
            && max_known < known
            //          #endif // SLOW_BASE
        ) {
          max_row = row;
          max_col = col;
          max_known = known;
        }
      }
    }
  } while (advanced);

  if (max_known == -1) {
    grid.show();
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
  assert(trial == DIM);
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

int main(int argc, char *argv[]) {
  freopen("/home/mike/workspace/parallel_lab/project-sudoku/data/16grid.txt",
          "r", stdin);
  Engine eng;
  Grid grid;
  read_grid(grid.data());

  grid.show();
  for (int i = 0; i < 20; ++i) {
    eng.candidate.push_back(grid);
    solve(eng);
  }
  auto beg_time = high_resolution_clock::now();
  constexpr int REP = 200;
  for (int i = 0; i < REP; ++i) {
    eng.candidate.clear();
    eng.candidate.push_back(grid);
    solve(eng);
  }
  auto end_time = high_resolution_clock::now();
  auto time =
      duration_cast<duration<double, std::milli>>(end_time - beg_time).count();
  cout << "average time: " << time / REP << "ms" << endl;
}
