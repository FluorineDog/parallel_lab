/************************************************************************
 * Solver
 *
 * The solver works using recursive backtracking. The general idea is to
 * find the cell with the smallest possible number of candidate values, and
 * to try each candidate, recursively solving until we find a solution.
 *
 * However, in cases where a cell has multiple candidates, we also consider
 * set-oriented backtracking -- choosing a value and trying each candidate
 * position. If this yields a smaller branching factor (it often eliminates
 * the need for backtracking), we try it instead.
 *
 * We keep searching until we've either found two solutions (demonstrating
 * that the grid does not have a unique solution), or we exhaust the search
 * tree.
 *
 * We also calculate a branch-difficulty score:
 *
 *    Sigma [(B_i - 1) ** 2]
 *
 * Where B_i are the branching factors at each node in the search tree
 * following the path from the root to the solution. A puzzle that could
 * be solved without backtracking has a branch-difficulty of 0.
 *
 * The final difficulty is:
 *
 *    Difficulty = B * C + E
 *
 * Where B is the branch-difficulty, E is the number of empty cells, and C
 * is the first power of ten greater than the number of elements.
 */

struct solve_context {
	uint8_t problem[ELEMENTS];
	int count;
	uint8_t *solution;
	int branch_score;
};

static void solve_recurse(struct solve_context *ctx, const set_t *freedom,
													int diff) {
	set_t new_free[ELEMENTS];
	set_t mask;
	int r;
	int i;
	int bf;

	r = search_least_free(ctx->problem, freedom);
	if (r < 0) {
		if (!ctx->count) {
			ctx->branch_score = diff;
			if (ctx->solution)
				memcpy(ctx->solution, ctx->problem,
							 ELEMENTS * sizeof(ctx->solution[0]));
		}

		ctx->count++;
		return;
	}

	mask = freedom[r];

	/* Otherwise, fall back to cell-oriented backtracking. */
	bf = count_bits(mask) - 1;
	diff += bf * bf;

	for (i = 0; i < DIM; i++)
		if (mask & (1 << i)) {
			memcpy(new_free, freedom, sizeof(new_free));
			freedom_eliminate(new_free, r % DIM, r / DIM, i + 1);
			ctx->problem[r] = i + 1;
			solve_recurse(ctx, new_free, diff);

			if (ctx->count >= 2) return;
		}

	ctx->problem[r] = 0;
}

#include <iostream>
#include <chrono>
using std::cout;
using std::endl;
using std::cerr;
using namespace std::chrono;

static int solve(const uint8_t *problem, uint8_t *solution, int *diff) {
	struct solve_context ctx;
	set_t freedom[ELEMENTS];

	memcpy(ctx.problem, problem, sizeof(ctx.problem));
	ctx.count = 0;
	ctx.branch_score = 0;
	ctx.solution = solution;

	init_freedom(problem, freedom);
	if (sanity_check(problem, freedom) < 0) return -1;

	solve_recurse(&ctx, freedom, 0);
	
	/* Calculate a difficulty score */
	if (diff) {
		int empty = 0;
		int mult = 1;
		int i;

		for (i = 0; i < ELEMENTS; i++)
			if (!problem[i]) empty++;

		while (mult <= ELEMENTS) mult *= 10;

		*diff = ctx.branch_score * mult + empty;
	}

	return ctx.count - 1;
}

