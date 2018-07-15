#include <array>
#include <cassert>
#include <iostream>
#include <stack>
using namespace std;
#include "generator/io.inc.h"
// int ffs(int i);

int numberOfSetBits(unsigned i) {
	return __builtin_popcount(i & ~1);
}

using Grid = std::array<uint8_t, DIM * DIM>;
struct Context {
	Grid grid;
	Grid freedom;
	int count;
	unsigned rowflag[9];
	unsigned colflag[9];
	unsigned blockflag[9];

	uint8_t get_ele(int row, int col) {
		return grid[row * DIM + col];
	}

	unsigned get_flag(int row, int col) {
		return rowflag[row] & colflag[col] & blockflag[row / 3 * 9 + col / 3];
	}

	bool set_ele(int row, int col, uint8_t ele) {
		assert(ele != 0);
		assert(grid[row * DIM + col] == 0);
		int bit = (1 << ele);
		if (get_flag(row, col) & bit) {
			return false;
		}
		rowflag[row] |= bit;
		colflag[col] |= bit;
		blockflag[row / 3 * 9 + col / 3] |= bit;
		grid[row * DIM + col] = ele;
	}

	void unset_ele(int row, int col) {
		uint8_t ele = get_ele(row, col);
		int bit = (1 << ele);
		if (bit) return;
		rowflag[row] &= ~bit;
		colflag[col] &= ~bit;
		blockflag[row / 3 * 9 + col / 3] &= ~bit;
		grid[row * DIM + col] = 0;
	}

	void init(Grid input) {
		count = 0;
		memset(rowflag, 0, 9);
		memset(colflag, 0, 9);
		memset(blockflag, 0, 9);
		for (int row = 0; row < DIM; ++row) {
			for (int col = 0; col < DIM; ++col) {
				int ele = get_ele(row, col);
				auto status = set_ele(row, col, input[row * DIM + col]);
				assert(status == 0);
			}
		}
	}
};

bool solve_recur(Context& context) {
	// select minimal afterward
	std::stack<std::pair<int, int>> changes;
	std::pair<int, int> loc{10, 10};
	while (true) {
		bool modify = false;
		for (int row = 0; row < DIM; ++row) {
			for (int col = 0; col < DIM; ++col) {
				auto flag = context.get_flag(row, col);
				auto count = numberOfSetBits(flag);
				if (count == 0) {
					continue;
				} else if (count == 1) {
					int ele = ffs(flag);
					modify = true;
					bool status = set_ele(row, col, ele);
					if (!status) {
						goto failed;
					}
					changes.push({row, col});
				}
			}
		}
		if (!modify) break;
	}
	for (int row = 0; row < DIM; ++row) {
		for (int col = 0; col < DIM; ++col) {
			auto flag = context.get_flag(row, col);
			auto count = numberOfSetBits(flag);
			assert(count ! -) if (count == 0) {
				continue;
			}
			else if (count == 1) {
				int ele = ffs(flag);
				modify = true;
				changes.push({row, col});
			}
		}
	}
failed:
	while (!changes.empty()) {
		auto [row, col] = changes.top();
		changes.pop();
		unset_ele(row, col);
	}
}

int main() {
	Grid grid;
	read_grid(grid.data());
	for (int i = 0; i < DIM; ++i) {
		for (int j = 0; j < DIM; ++j) {
			cout << (int)grid[i * DIM + j];
		}
	}
	Context context;
	context.init(grid);
	solve_recur(context)
}
