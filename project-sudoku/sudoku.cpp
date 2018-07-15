#include <iostream>
using namespace std;
#include "generator/io.inc.h"

int main() {
	uint8_t grid[DIM * DIM];
	read_grid(grid);
	for (int i = 0; i < DIM; ++i) {
		for (int j = 0; j < DIM; ++j) {
      cout << (int)grid[i*DIM + j];
		}
    cout << endl;
	}
}
