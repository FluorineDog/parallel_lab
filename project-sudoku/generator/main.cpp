#define NO_SOFA
#include "io.inc.h"
#include "analysis.inc.h"
#include "solver.inc.h"
#include "generator.inc.h"


/************************************************************************
 * Puzzle generator
 *
 * To generate a puzzle, we start with a solution grid, and an initial
 * puzzle (which may be the same as the solution). We try altering the
 * puzzle by either randomly adding a pair of clues from the solution, or
 * randomly removing a pair of clues. After each alteration, we check to
 * see if we have a valid puzzle. If it is, and it's more difficult than
 * anything we've encountered so far, save it as the best puzzle.
 *
 * To avoid getting stuck in local minima in the space of puzzles, we allow
 * the algorithm to wander for a few steps before starting again from the
 * best-so-far puzzle.
 */

static int harden_puzzle(const uint8_t *solution, uint8_t *puzzle, int max_iter,
												 int max_score, int target_score) {
	int best = 0;
	int i;

	solve(puzzle, NULL, &best);

	for (i = 0; i < max_iter; i++) {
		uint8_t next[ELEMENTS];
		int j;

		memcpy(next, puzzle, sizeof(next));

		for (j = 0; j < DIM * 2; j++) {
			int c = random() % ELEMENTS;
			int s;

			if (random() & 1) {
				next[c] = solution[c];
				next[ELEMENTS - c - 1] = solution[ELEMENTS - c - 1];
			} else {
				next[c] = 0;
				next[ELEMENTS - c - 1] = 0;
			}

			if (!solve(next, NULL, &s) && s > best &&
					(s <= max_score || max_score < 0)) {
				memcpy(puzzle, next, sizeof(puzzle[0]) * ELEMENTS);
				best = s;

				if (target_score >= 0 && s >= target_score) return best;
			}
		}
	}

	return best;
}

/************************************************************************
 * Command-line user interface
 */

struct options {
	int max_iter;
	int target_diff;
	int max_diff;
	const struct gridline_def *gl_def;
	const char *action;
};

static void usage(const char *progname) {
	printf(
			"usage: %s [options] <action>\n\n"
			"Options may be any of the following:\n"
			"    -i <iterations>    Maximum iterations for puzzle generation.\n"
			"    -m <score>         Maximum difficulty for puzzle generation.\n"
			"    -t <score>         Target difficulty for puzzle generation.\n"
			"    -u                 Use UTF-8 line-drawing characters.\n"
			"    -a                 Use ASCII line-drawing characters.\n"
			"\n"
			"Action should be one of:\n"
			"    solve              Read a grid from stdin and solve it.\n"
			"    examine            Read a grid and estimate the difficulty.\n"
			"    print              Read a grid and reformat it.\n"
			"    generate           Generate and print a new puzzle.\n"
			"    harden             Read an existing puzzle and make it harder.\n"
			"    gen-grid           Generate a valid grid.\n",
			progname);
}

static void version(void) {
	printf(
			"Sudoku puzzle generator, 10 Jun 2011\n"
			"Copyright (C) 2011 Daniel Beer <dlbeer@gmail.com>\n"
			"\n"
			"Permission to use, copy, modify, and/or distribute this software for "
			"any\n"
			"purpose with or without fee is hereby granted, provided that the above\n"
			"copyright notice and this permission notice appear in all copies.\n"
			"\n"
			"THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL "
			"WARRANTIES\n"
			"WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF\n"
			"MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE "
			"FOR\n"
			"ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES\n"
			"WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN\n"
			"ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT "
			"OF\n"
			"OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.\n");
}

static int parse_options(int argc, char **argv, struct options *o) {
	const static struct option longopts[] = {
			{"help", 0, 0, 'H'}, {"version", 0, 0, 'V'}, {NULL, 0, 0, 0}};
	int i;

	memset(o, 0, sizeof(*o));
#if ORDER <= 3
	o->max_iter = 200;
#else
	o->max_iter = 20;
#endif
	o->target_diff = -1;
	o->max_diff = -1;

	while ((i = getopt_long(argc, argv, "i:t:m:ua", longopts, NULL)) >= 0)
		switch (i) {
			case 'i':
				o->max_iter = atoi(optarg);
				break;

			case 't':
				o->target_diff = atoi(optarg);
				break;

			case 'm':
				o->max_diff = atoi(optarg);
				break;

			case 'u':
				o->gl_def = &utf8_def;
				break;

			case 'a':
				o->gl_def = &ascii_def;
				break;

			case 'H':
				usage(argv[0]);
				exit(0);
				break;

			case 'V':
				version();
				exit(0);

			case '?':
				fprintf(stderr,
								"Try --help for usage "
								"information.\n");
				return -1;
		}

	argc -= optind;
	argv += optind;

	if (argc < 1) {
		version();
		printf("\n");
		fprintf(stderr,
						"You need to specify an action. "
						"Try --help.\n");
		return -1;
	}

	o->action = argv[0];
	return 0;
}

static void print_grid_opt(const struct options *o, const uint8_t *grid) {
	if (o->gl_def)
		print_grid_pretty(o->gl_def, grid);
	else
		print_grid(grid);
}
#include <chrono>
using namespace std::chrono;
static int action_se(const struct options *o) {
	uint8_t grid[ELEMENTS];
	uint8_t solution[ELEMENTS];
	int diff;
	int r;
	if (read_grid(grid) < 0) return -1;

	
	auto t0 = high_resolution_clock::now();
	r = solve(grid, solution, &diff);
	auto t1 = high_resolution_clock::now();
	auto t = duration_cast<duration<double, std::milli>>(t1-t0).count();

	if (r < 0) {
		printf("Grid is unsolvable\n");
		return -1;
	}

	if (*o->action == 's' || *o->action == 'S') {
		print_grid_opt(o, solution);
		printf("\n");
	}

	if (r > 0) {
		printf("Solution is not unique\n");
		return -1;
	}

	printf("Unique solution. Difficulty: %d\n", diff);
	printf("duration: %lf ms\n", t);
	return 0;
}

static int action_print(const struct options *o) {
	uint8_t grid[ELEMENTS];

	if (read_grid(grid) < 0) return -1;

	print_grid_opt(o, grid);
	return 0;
}

static int action_gen_grid(const struct options *o) {
	uint8_t grid[ELEMENTS];

	choose_grid(grid);
	print_grid_opt(o, grid);
	return 0;
}

static int action_harden(const struct options *o) {
	uint8_t solution[ELEMENTS];
	uint8_t grid[ELEMENTS];
	int r;
	int old_diff;
	int new_diff;

	if (read_grid(grid) < 0) return -1;

	r = solve(grid, solution, &old_diff);
	if (r < 0) {
		printf("Grid is unsolvable\n");
		return -1;
	}

	if (r) memcpy(grid, solution, sizeof(grid[0]) * ELEMENTS);

	new_diff =
			harden_puzzle(solution, grid, o->max_iter, o->max_diff, o->target_diff);

	print_grid_opt(o, grid);
	printf("\nDifficulty: %d\n", new_diff);

	if (r)
		printf("Original puzzle was not uniquely solvable\n");
	else
		printf("Original difficulty: %d\n", old_diff);

	return 0;
}

static int action_generate(const struct options *o) {
	uint8_t puzzle[ELEMENTS];
	uint8_t grid[ELEMENTS];
	int diff;

	choose_grid(grid);
	memcpy(puzzle, grid, ELEMENTS * sizeof(puzzle[0]));

	diff = harden_puzzle(grid, puzzle, o->max_iter, o->max_diff, o->target_diff);
	print_grid_opt(o, puzzle);
	printf("\nDifficulty: %d\n", diff);
	return 0;
}

struct action {
	const char *name;
	int (*func)(const struct options *o);
};

static const struct action actions[] = {{"solve", action_se},
																				{"examine", action_se},
																				{"print", action_print},
																				{"gen-grid", action_gen_grid},
																				{"harden", action_harden},
																				{"generate", action_generate},
																				{NULL, NULL}};

int main(int argc, char **argv) {
	struct options o;
	const struct action *a = actions;

	if (parse_options(argc, argv, &o) < 0) return -1;

	while (a->name && strcasecmp(o.action, a->name)) a++;

	if (!a->name) {
		fprintf(stderr, "Unknown action: %s. Try --help.\n", o.action);
		return -1;
	}

	srandom(time(NULL));
	return a->func(&o);
}
