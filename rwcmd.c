/*
  Read/Write commands

  Copyright (C) 2018 David Bulkow

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcimem.h"

static int read_range(struct state *state, int argc, char **argv) {
	char *ep;
	unsigned int loc;
	unsigned int len;

	if (argc < 3)
		return 1;

	loc = strtoul(argv[1], &ep, state->radix);
	if (*ep != '\0') {
		fprintf(stderr, "location not valid number\n");
		return 0;
	}

	len = strtoul(argv[2], &ep, state->radix);
	if (*ep != '\0') {
		fprintf(stderr, "length not valid number\n");
		return 0;
	}

	if (loc + len > state->maplen)
		len -= (loc + len) - state->maplen;

	hexdumpn(state->hex_format, state->map + loc, len);

	return 0;
}

static int read_one(struct state *state, int argc, char **argv) {
	unsigned int loc;
	char *ep;

	if (argc < 2)
		return 1;

	loc = strtoul(argv[1], &ep, state->radix);
	if (*ep != '\0') {
		fprintf(stderr, "location not valid number\n");
		return 0;
	}

	switch (argv[0][1]) {
	case '1':
	case '2':
	case '4':
	case '8':
		printf("%c %d\n", argv[0][1], loc);
		break;
	default:
		return 1;
	}

	return 0;
}

static int write_one(struct state *state, int argc, char **argv) {
	unsigned int loc;
	unsigned int val;
	char *ep;

	if (argc < 3)
		return 1;

	loc = strtoul(argv[1], &ep, state->radix);
	if (*ep != '\0') {
		fprintf(stderr, "location not valid number\n");
		return 0;
	}

	val = strtoul(argv[2], &ep, state->radix);
	if (*ep != '\0') {
		fprintf(stderr, "location not valid number\n");
		return 0;
	}

	switch (argv[0][1]) {
	case '1':
	case '2':
	case '4':
	case '8':
		printf("%c %d %d\n", argv[0][1], loc, val);
		break;
	default:
		return 1;
	}

	return 0;
}

static struct {
	char *name;
	int (*func)(struct state *state, int argc, char **argv);
} rw[] = {
	{"rd", read_range},
	{"r1", read_one},
	{"r2", read_one},
	{"r4", read_one},
	{"r8", read_one},
	{"w1", write_one},
	{"w2", write_one},
	{"w4", write_one},
	{"w8", write_one},
};

int rwcmd(struct state *state, int argc, char **argv) {
	int rv = 0;
	int i;

	for (i = 0; i < nelem(rw); i++)
		if (strcmp(argv[0], rw[i].name) == 0)
			rv = rw[i].func(state, argc, argv);

	if (rv == 0)
		return 0;

	printf("rd <loc> <len>       read range\n");
	printf("r1 <loc>             read byte\n");
	printf("r2 <loc>             read word\n");
	printf("r4 <loc>             read double-word\n");
	printf("r8 <loc>             read quad-word\n");
	printf("w1 <loc> <val>       write byte\n");
	printf("w2 <loc> <val>       write word\n");
	printf("w4 <loc> <val>       write double-word\n");
	printf("w8 <loc> <val>       write quad-word\n");

	return 0;
}
