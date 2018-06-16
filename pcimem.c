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
#include <unistd.h>
#include <stdlib.h>

#include "pcimem.h"

void usage(char *cmd) {
	fprintf(stderr, "usage: %s [-p <domain>] -s <bus>:<dev>:<func>\n", cmd);
	fprintf(stderr, "       -h help\n");
	fprintf(stderr, "       -d pci domain [default 0]\n");
	fprintf(stderr, "       -s pci device bus:dev:func\n");
	exit(1);
}

int main(int argc, char **argv) {
	int opt;
	char *pcidev = NULL;
	int pcidomain = 0;

	while ((opt = getopt(argc, argv, "hs:d:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
		case 's':
			pcidev = optarg;
			break;
		case 'd':
			pcidomain = atoi(optarg);
			break;
		default:
			usage(argv[0]);
		}
	}

	command(pcidev, pcidomain);

	return 0;
}
