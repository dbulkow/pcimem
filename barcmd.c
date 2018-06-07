/*
  PCI BAR looking, setting, open, close

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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>

#include "pcimem.h"

int openres(char *path) {
	int fd;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		printf("open(%s): %s\n", path, strerror(errno));
		return -1;
	}

	return fd;
}

int closeres(int fd) {
	return close(fd);
}

int listbar(char *pcidev) {
	DIR *dir;
	struct dirent *dent;
	char path[PATH_MAX];

	if (strlen(pcidev) == 0) {
		printf("no pci device configured\n");
		return 0;
	}

	snprintf(path, sizeof(path), "%s/%s", devicedir, pcidev);

	dir = opendir(path);
	if (dir == NULL) {
		printf("opendir(%s): %s\n", path, strerror(errno));
		return 0;
	}

	while ((dent = readdir(dir)) != NULL) {
		if (strstr(dent->d_name, "resource") == dent->d_name &&
		    strlen(dent->d_name) != strlen("resource")) {
			printf("%s\n", dent->d_name);
		}
	}

	closedir(dir);

	return 0;

}

int barcmd(struct state *state, int argc, char **argv) {
	char path[PATH_MAX];
	int res;

	if (argc == 1)
		return listbar(state->pcidev);

	if (argc > 1)
		res = atoi(argv[1]);

	snprintf(path, sizeof(path), "%s/%s/resource%d", devicedir, state->pcidev, res);

	state->res = -1;

	if (state->fd >= 0)
		closeres(state->fd);

	state->fd = openres(path);
	if (state->fd < 0)
		return 0;

	state->res = res;

	return 0;
}
