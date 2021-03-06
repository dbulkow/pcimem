/*
  PCI BAR lookup, setting, open, close

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
#include <sys/mman.h>

#include "pcimem.h"

int openres(struct state *state, char *path) {
	struct stat st;
	void *base;
	int fd;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		printf("open(%s): %s\n", path, strerror(errno));
		return -1;
	}

	if (fstat(fd, &st) < 0) {
		printf("fstat(%s): %s\n", path, strerror(errno));
		close(fd);
		return -1;
	}

	base = mmap(0, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (base == (void *) -1) {
		printf("mmap(%s): %s\n", path, strerror(errno));
		close(fd);
		return -1;
	}

	state->maplen = st.st_size;
	state->map = base;

	return fd;
}

int closeres(struct state *state) {
	int ret;

	if (state->map != NULL && munmap(state->map, state->maplen) < 0) {
		printf("munmap: %s\n", strerror(errno));
		ret = -1;
	}
	if (state->fd >= 0 && close(state->fd)) {
		printf("close: %s\n", strerror(errno));
		ret = -1;
	}

	state->map = NULL;
	state->maplen = 0;
	state->fd = -1;

	return ret;
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
			struct stat st;
			char statpath[PATH_MAX];
			int size = 0;

			snprintf(statpath, sizeof(statpath), "%s/%s", path, dent->d_name);

			if (stat(statpath, &st) < 0)
				printf("stat(%s): %s\n", statpath, strerror(errno));
			else
				size = st.st_size;

			printf("%s %d [%8.8x] bytes\n", dent->d_name, size, size);
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
		closeres(state);

	state->fd = openres(state, path);
	if (state->fd < 0)
		return 0;

	state->res = res;

	return 0;
}
