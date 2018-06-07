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

void usage(char *cmd) {
	fprintf(stderr, "usage: %s [-l] -r <resource #> [-p <domain>] -s <bus>:<dev>:<func>\n", cmd);
	fprintf(stderr, "       -h help\n");
	fprintf(stderr, "       -l list resources for device\n");
	fprintf(stderr, "       -r specify resource number\n");
	fprintf(stderr, "       -p pci domain - typically 0\n");
	fprintf(stderr, "       -s pci device bus:dev:func\n");
	exit(1);
}

int main(int argc, char **argv) {
	int flags, opt;
	char *pcidev;
	int pcidomain = 0;
	char pci[1024];
	int res;
	int res_set = 0;
	int dolist = 0;

	while ((opt = getopt(argc, argv, "hlr:s:p:")) != -1) {
		switch (opt) {
		case 'h':
			usage(argv[0]);
		case 'l':
			dolist = 1;
			break;
		case 's':
			pcidev = optarg;
			break;
		case 'p':
			pcidomain = atoi(optarg);
			break;
		case 'r':
			res = atoi(optarg);
			res_set = 1;
			break;
		default:
			usage(argv[0]);
		}
	}

	if (pcidev == NULL)
		usage(argv[0]);

	sprintf(pci, "%04d:%s", pcidomain, pcidev);

	if (dolist) {
		findres(pci);
		exit(0);
	}

	if (res_set != 1)
		usage(argv[0]);

	command(pci, res);

	return 0;
}
