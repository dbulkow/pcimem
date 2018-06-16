/*
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
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "pcimem.h"

void usage(char *cmd) {
	fprintf(stderr, "usage: %s [-p <domain>] -s <bus>:<dev>:<func>\n", cmd);
	fprintf(stderr, "       -h help\n");
	fprintf(stderr, "       -d <pci domain> [default 0]\n");
	fprintf(stderr, "       -s <pci device bus:dev:func>\n");
	fprintf(stderr, "       -f <bar #>\n");
	fprintf(stderr, "       -r <radix> from [2, 10, 16]\n");
	exit(1);
}

int main(int argc, char **argv) {
	int opt;
	struct state state;
	int domain = 0;
	char *pcidev = NULL;

	state.res = -1;
	state.pcidev[0] = '\0';
	state.fd = -1;
	state.map = NULL;
	state.maplen = -1;
	state.radix = 0;

	while ((opt = getopt(argc, argv, "hs:d:b:r:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
		case 's':
			pcidev = optarg;
			break;
		case 'd':
			domain = atoi(optarg);
			break;
		case 'b':
			state.res = atoi(optarg);
			break;
		case 'r':
			state.radix = atoi(optarg);
			break;
		default:
			usage(argv[0]);
		}
	}

	switch (state.radix) {
	case 2:
	case 0: // 10
	case 16:
		break;
	default:
		fprintf(stderr, "invalid radix\n");
		return 1;
	}

	if (pcidev)
		snprintf("%04d:%s", domain, pcidev);

	command(&state);

	return 0;
}
