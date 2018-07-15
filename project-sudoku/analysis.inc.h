/************************************************************************
 * Cell freedom analysis.
 *
 * Cell freedoms are sets, represented by bitfields. If bit N (counting
 * with LSB = 0) is set, then value (N+1) is present in the set.
 *
 * A cell freedom analysis results in a grid of sets, giving the immediately
 * allowable values in each cell position.
 *
 * If possible, it's cheaper to generate a freedom map for a new position by
 * modifying the previous position's freedom map, rather than rebuilding
 * from scratch.
 *
 * The search_least_free() function searches for the empty cell with the
 * smallest number of candidate values. It returns -1 if no empty cell was
 * found -- meaning that the grid is already solved.
 *
 * If the freedom for an empty cell is 0, this indicates that the grid is
 * unsolvable.
 */
typedef uint16_t set_t;

#define SINGLETON(v) (1 << ((v)-1))
#define ALL_VALUES ((1 << DIM) - 1)

static int count_bits(int x) {
	int count = 0;

	while (x) {
		x &= x - 1;
		count++;
	}

	return count;
}

static void freedom_eliminate(set_t *freedom, int x, int y, int v) {
	set_t mask = ~SINGLETON(v);
	int b;
	int i, j;
	set_t saved = freedom[y * DIM + x];

	b = x;
	for (i = 0; i < DIM; i++) {
		freedom[b] &= mask;
		b += DIM;
	}

	b = y * DIM;
	for (i = 0; i < DIM; i++) freedom[b + i] &= mask;

	b = (y - y % ORDER) * DIM + x - x % ORDER;
	for (i = 0; i < ORDER; i++) {
		for (j = 0; j < ORDER; j++) freedom[b + j] &= mask;

		b += DIM;
	}

	freedom[y * DIM + x] = saved;
}

static void init_freedom(const uint8_t *problem, set_t *freedom) {
	int x, y;

	for (x = 0; x < ELEMENTS; x++) freedom[x] = ALL_VALUES;

	for (y = 0; y < DIM; y++)
		for (x = 0; x < DIM; x++) {
			int v = problem[y * DIM + x];

			if (v) freedom_eliminate(freedom, x, y, v);
		}
}

static int sanity_check(const uint8_t *problem, const set_t *freedom) {
	int i;

	for (i = 0; i < ELEMENTS; i++) {
		int v = problem[i];

		if (v) {
			set_t f = freedom[i];

			if (!(f & SINGLETON(v))) return -1;
		}
	}

	return 0;
}

static int search_least_free(const uint8_t *problem, const set_t *freedom) {
	int i;
	int best_index = -1;
	int best_score = -1;

	for (i = 0; i < ELEMENTS; i++) {
		int v = problem[i];

		if (!v) {
			int score = count_bits(freedom[i]);

			if (best_score < 0 || score < best_score) {
				best_index = i;
				best_score = score;
			}
		}
	}

	return best_index;
}


/************************************************************************
 * Set-oriented freedom analysis.
 *
 * In normal freedom analysis, we find candidate values for each cell. In
 * set-oriented freedom analysis, we find candidate cells for each value.
 * There are 3 * DIM sets to consider (DIM boxes, rows and columns).
 *
 * The sofa() function returns the size of the smallest set of positions
 * found, along with a list of those positions and a value which can occupy
 * all of those positions. It returns -1 if a set of positions couldn't
 * be found.
 *
 * If it returns 0, this indicates that there exists a set, missing a value,
 * with no possible positions for that value -- the grid is therefore
 * unsolvable.
 *
 * If the program is compiled with -DNO_SOFA, this analysis is not used.
 */

#ifndef NO_SOFA
struct sofa_context {
	const uint8_t *grid;
	const set_t *freedom;

	int best[DIM];
	int best_size;
	int best_value;
};

static void sofa_set(struct sofa_context *ctx, const int *set) {
	int count[DIM];
	int i;
	int best = -1;
	set_t missing = ALL_VALUES;

	/* Find out what's missing from the set, and how many available
	 * slots for each missing number.
	 */
	memset(count, 0, sizeof(count));
	for (i = 0; i < DIM; i++) {
		int v = ctx->grid[set[i]];

		if (v) {
			missing &= ~SINGLETON(v);
		} else {
			set_t freedom = ctx->freedom[set[i]];
			int j;

			for (j = 0; j < DIM; j++)
				if (freedom & (1 << j)) count[j]++;
		}
	}

	/* Look for the missing number with the fewest available slots. */
	for (i = 0; i < DIM; i++)
		if ((missing & (1 << i)) && (best < 0 || count[i] < count[best])) best = i;

	/* Did we find anything? */
	if (best < 0) return;

	/* If it's better than anything we've found so far, save the result */
	if (ctx->best_size < 0 || count[best] < ctx->best_size) {
		int j = 0;
		set_t mask = 1 << best;

		ctx->best_value = best + 1;
		ctx->best_size = count[best];

		for (i = 0; i < DIM; i++)
			if (!ctx->grid[set[i]] && (ctx->freedom[set[i]] & mask))
				ctx->best[j++] = set[i];
	}
}

static int sofa(const uint8_t *grid, const set_t *freedom, int *set,
								int *value) {
	struct sofa_context ctx;
	int i;

	memset(&ctx, 0, sizeof(ctx));
	ctx.grid = grid;
	ctx.freedom = freedom;
	ctx.best_size = -1;
	ctx.best_value = -1;

	for (i = 0; i < DIM; i++) {
		int b = (i / ORDER) * ORDER * DIM + (i % ORDER) * ORDER;
		int set[DIM];
		int j;

		for (j = 0; j < DIM; j++) set[j] = j * DIM + i;
		sofa_set(&ctx, set);

		for (j = 0; j < DIM; j++) set[j] = i * DIM + j;
		sofa_set(&ctx, set);

		for (j = 0; j < DIM; j++) set[j] = b + (j / ORDER) * DIM + j % ORDER;
		sofa_set(&ctx, set);
	}

	memcpy(set, ctx.best, sizeof(ctx.best));
	*value = ctx.best_value;
	return ctx.best_size;
}
#endif

