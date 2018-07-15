/* Sudoku puzzle generator
 * Copyright (C) 2011 Daniel Beer <dlbeer@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ctype.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Compile this program with:
 *
 *    gcc -O3 -Wall -o sugen sugen.c
 *
 * The puzzle order corresponds to regular Sudoku by default (3), but
 * you can override it by specifying -DORDER=x at compile time. Orders
 * from 1 through 4 are supported.
 */

#ifndef ORDER
#define ORDER 3
#endif

#define DIM (ORDER * ORDER)
#define ELEMENTS (DIM * DIM)

/************************************************************************
 * Parsing and printing
 *
 * The parser is quite forgiving and designed so that it can also parse
 * grids produced by the pretty-printer.
 */

struct hline_def {
	const char *start;
	const char *mid;
	const char *end;
	const char *major;
	const char *minor;
};

struct gridline_def {
	struct hline_def top;
	struct hline_def major;
	struct hline_def minor;
	struct hline_def bottom;
	const char *vl_major;
	const char *vl_minor;
};

const static struct gridline_def ascii_def = {
		.top = {.start = "+", .mid = "-", .end = "+", .major = "+", .minor = "-"},
		.major = {.start = "+", .mid = "-", .end = "+", .major = "+", .minor = "-"},
		.minor = {.start = "|", .mid = ".", .end = "|", .major = "|", .minor = ":"},
		.bottom =
				{.start = "+", .mid = "-", .end = "+", .major = "+", .minor = "-"},
		.vl_major = "|",
		.vl_minor = ":"};

const static struct gridline_def utf8_def = {
		.top = {.start = "\xe2\x95\x94",
						.mid = "\xe2\x95\x90",
						.end = "\xe2\x95\x97",
						.major = "\xe2\x95\xa6",
						.minor = "\xe2\x95\xa4"},
		.major = {.start = "\xe2\x95\xa0",
							.mid = "\xe2\x95\x90",
							.end = "\xe2\x95\xa3",
							.major = "\xe2\x95\xac",
							.minor = "\xe2\x95\xaa"},
		.minor = {.start = "\xe2\x95\x9f",
							.mid = "\xe2\x94\x80",
							.end = "\xe2\x95\xa2",
							.major = "\xe2\x95\xab",
							.minor = "\xe2\x94\xbc"},
		.bottom = {.start = "\xe2\x95\x9a",
							 .mid = "\xe2\x95\x90",
							 .end = "\xe2\x95\x9d",
							 .major = "\xe2\x95\xa9",
							 .minor = "\xe2\x95\xa7"},
		.vl_major = "\xe2\x95\x91",
		.vl_minor = "\xe2\x94\x82"};

static int read_grid(uint8_t *grid) {
	int x = 0;
	int y = 0;
	int c;
	int can_skip = 0;

	memset(grid, 0, sizeof(grid[0]) * ELEMENTS);

	while ((c = getchar()) >= 0) {
		if (c == '\n') {
			if (x > 0) y++;
			x = 0;
			can_skip = 0;
		} else if (c == '.' || c == '-') {
			can_skip = 0;
		} else if (c == '_' || c == '0') {
			x++;
		} else if (c == 0x82 || c == 0x91 || c == '|' || c == ':') {
			if (can_skip) x++;
			can_skip = 1;
		} else if (isalnum(c) && x < DIM && y < DIM) {
			int v;

			if (isdigit(c))
				v = c - '0';
			else if (isupper(c))
				v = c - 'A' + 10;
			else
				v = c - 'a' + 10;

			if (v >= 1 && v <= DIM) {
				grid[y * DIM + x] = v;
				x++;
				can_skip = 0;
			}
		}
	}

	if ((y <= DIM - 1) || ((y == DIM - 1) && (x < DIM))) {
		fprintf(stderr,
						"Too few cells in grid. Input ran out at "
						"position (%d, %d)\n",
						x, y);
		return -1;
	}

	return 0;
}

static const char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static void print_grid(const uint8_t *grid) {
	int y;

	for (y = 0; y < DIM; y++) {
		int x;

		for (x = 0; x < DIM; x++) {
			int v = grid[y * DIM + x];

			if (x) printf(" ");

			if (v)
				printf("%c", alphabet[v]);
			else
				printf("_");
		}

		printf("\n");
	}
}

static void draw_hline(const struct hline_def *def) {
	int i;

	printf("%s", def->start);
	for (i = 0; i < DIM; i++) {
		printf("%s%s%s", def->mid, def->mid, def->mid);

		if (i + 1 < DIM) printf("%s", ((i + 1) % ORDER) ? def->minor : def->major);
	}
	printf("%s\n", def->end);
}

static void print_grid_pretty(const struct gridline_def *def,
															const uint8_t *grid) {
	int y;

	draw_hline(&def->top);

	for (y = 0; y < DIM; y++) {
		int x;

		for (x = 0; x < DIM; x++) {
			int v = grid[y * DIM + x];

			if (x % ORDER)
				printf("%s", def->vl_minor);
			else
				printf("%s", def->vl_major);

			if (v)
				printf(" %c ", alphabet[v]);
			else
				printf("   ");
		}

		printf("%s\n", def->vl_major);

		if (y + 1 < DIM) {
			if ((y + 1) % ORDER)
				draw_hline(&def->minor);
			else
				draw_hline(&def->major);
		}
	}

	draw_hline(&def->bottom);
}

