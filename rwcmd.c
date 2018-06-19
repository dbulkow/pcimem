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
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "pcimem.h"

static int read_range(struct state *state, int argc, char **argv) {
	char *ep;
	unsigned int loc;
	unsigned int len;
	char *map = NULL;

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

	if (state->res == -1) {
		if (state->cfgfd >= 0) {
			int n;
			map = malloc(len);
			n = pread(state->cfgfd, map, len, loc);
			if (n < 0) {
				fprintf(stderr, "pread(%s): %s\n", state->pcidev, strerror(errno));
				return 0;
			}
			len = n;
		}
	} else {
		map = state->map + loc;
		if (loc + len > state->maplen)
			len -= (loc + len) - state->maplen;
	}

	if (map == NULL) {
		fprintf(stderr, "no data mapped\n");
		return 0;
	}

	hexdumpn(state->hex_format, map, len);

	if (state->res == -1 && map)
		free(map);

	return 0;
}

static int read_one(struct state *state, int argc, char **argv) {
	char *args[3];

	if (argc < 2)
		return 1;

	args[0] = "rd";
	args[1] = argv[1];
	args[2] = argv[0]+1;

	return read_range(state, 3, args);
}

static int write_cfg(struct state *state, unsigned int loc, unsigned int len, unsigned int val) {
	unsigned short *vs;
	unsigned long *vl;
	unsigned char data[4];
	ssize_t n;

	if (state->cfgfd == -1) {
		fprintf(stderr, "config space not open\n");
		return 0;
	}

	switch (len) {
	case 1:
		data[0] = val & 0xff;
		break;
	case 2:
		vs = (unsigned short *) data;
		*vs = val;
		break;
	case 4:
		vl = (unsigned long *) data;
		*vl = val;
		break;
	default:
		return 1;
	}

	n = pwrite(state->cfgfd, data, len, loc);
	if (n < 0) {
		fprintf(stderr, "pwrite(%s): %s\n", state->pcidev, strerror(errno));
		return 0;
	} else if (n != len) {
		fprintf(stderr, "short write(%s) got %d expected %d\n", state->pcidev, (int) n, len);
		return 0;
	}

	return 0;
}

static int write_one(struct state *state, int argc, char **argv) {
	unsigned int loc;
	unsigned int val;
	unsigned int len;
	char *ep;
	unsigned char *map;

	if (argc < 3)
		return 1;

	loc = strtoul(argv[1], &ep, state->radix);
	if (*ep != '\0') {
		fprintf(stderr, "location (%s) not valid number\n", argv[1]);
		return 0;
	}

	val = strtoul(argv[2], &ep, state->radix);
	if (*ep != '\0') {
		fprintf(stderr, "location (%s) not valid number\n", argv[2]);
		return 0;
	}

	len = argv[0][1] - '0';

	if (state->res == -1)
		return write_cfg(state, loc, len, val);

	if (loc + len > state->maplen)
		len -= (loc + len) - state->maplen;

	map = state->map;

	switch (len) {
	case 1:
		map[loc] = val;
		break;
	case 2:
		map[loc+0] = val & 0xff;
		map[loc+1] = (val >> 8) & 0xff;
		break;
	case 4:
		map[loc+0] = val & 0xff;
		map[loc+1] = (val >> 8) & 0xff;
		map[loc+2] = (val >> 16) & 0xff;
		map[loc+3] = (val >> 24) & 0xff;
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

	return 0;
}
