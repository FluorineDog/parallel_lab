/************************************************************************
 * Grid generator
 *
 * We generate grids using a backtracking algorithm similar to the basic
 * solver algorithm. At each step, choose a cell with the smallest number
 * of possible values, and try each value, solving recursively. The key
 * difference is that the values are tested in a random order.
 *
 * An empty grid can be initially populated with a large number of values
 * without backtracking. In the ORDER == 3 case, we can easily fill the
 * whole top band the the first column before resorting to backtracking.
 */

static int pick_value(set_t set) {
	int x = random() % count_bits(set);
	int i;

	for (i = 0; i < DIM; i++)
		if (set & (1 << i)) {
			if (!x) return i + 1;
			x--;
		}

	return 0;
}

static void choose_b1(uint8_t *problem) {
	set_t set = ALL_VALUES;
	int i, j;

	for (i = 0; i < ORDER; i++)
		for (j = 0; j < ORDER; j++) {
			int v = pick_value(set);

			problem[i * DIM + j] = v;
			set &= ~SINGLETON(v);
		}
}

#if ORDER == 3
static void choose_b2(uint8_t *problem) {
	set_t used[ORDER];
	set_t chosen[ORDER];
	set_t set_x, set_y;
	int i, j;

	memset(used, 0, sizeof(used));
	memset(chosen, 0, sizeof(chosen));

	/* Gather used values from B1 by box-row */
	for (i = 0; i < ORDER; i++)
		for (j = 0; j < ORDER; j++) used[i] |= SINGLETON(problem[i * DIM + j]);

	/* Choose the top box-row for B2 */
	set_x = used[1] | used[2];
	for (i = 0; i < ORDER; i++) {
		int v = pick_value(set_x);
		set_t mask = SINGLETON(v);

		chosen[0] |= mask;
		set_x &= ~mask;
	}

	/* Choose values for the middle box-row, as long as we can */
	set_x = (used[0] | used[2]) & ~chosen[0];
	set_y = (used[0] | used[1]) & ~chosen[0];

	while (count_bits(set_y) > 3) {
		int v = pick_value(set_x);
		set_t mask = SINGLETON(v);

		chosen[1] |= mask;
		set_x &= ~mask;
		set_y &= ~mask;
	}

	/* We have no choice for the remainder */
	chosen[1] |= set_x & ~set_y;
	chosen[2] |= set_y;

	/* Permute the triplets in each box-row */
	for (i = 0; i < ORDER; i++) {
		set_t set = chosen[i];
		int j;

		for (j = 0; j < ORDER; j++) {
			int v = pick_value(set);

			problem[i * DIM + j + ORDER] = v;
			set &= ~SINGLETON(v);
		}
	}
}

static void choose_b3(uint8_t *problem) {
	int i;

	for (i = 0; i < ORDER; i++) {
		set_t set = ALL_VALUES;
		int j;

		/* Eliminate already-used values in this row */
		for (j = 0; j + ORDER < DIM; j++) set &= ~SINGLETON(problem[i * DIM + j]);

		/* Permute the remaining values in the last box-row */
		for (j = 0; j < ORDER; j++) {
			int v = pick_value(set);

			problem[i * DIM + DIM - ORDER + j] = v;
			set &= ~SINGLETON(v);
		}
	}
}
#endif /* ORDER == 3 */

static void choose_col1(uint8_t *problem) {
	set_t set = ALL_VALUES;
	int i;

	for (i = 0; i < ORDER; i++) set &= ~SINGLETON(problem[i * DIM]);

	for (; i < DIM; i++) {
		int v = pick_value(set);

		problem[i * DIM] = v;
		set &= ~SINGLETON(v);
	}
}

static int choose_rest(uint8_t *grid, const set_t *freedom) {
	int i = search_least_free(grid, freedom);
	set_t set;

	if (i < 0) return 0;

	set = freedom[i];
	while (set) {
		set_t new_free[ELEMENTS];
		int v = pick_value(set);

		set &= ~SINGLETON(v);
		grid[i] = v;

		memcpy(new_free, freedom, sizeof(new_free));
		freedom_eliminate(new_free, i % DIM, i / DIM, v);

		if (!choose_rest(grid, new_free)) return 0;
	}

	grid[i] = 0;
	return -1;
}

static void choose_grid(uint8_t *grid) {
	set_t freedom[ELEMENTS];

	memset(grid, 0, sizeof(grid[0]) * ELEMENTS);

	choose_b1(grid);
#if ORDER == 3
	choose_b2(grid);
	choose_b3(grid);
#endif
	choose_col1(grid);

	init_freedom(grid, freedom);
	choose_rest(grid, freedom);
}