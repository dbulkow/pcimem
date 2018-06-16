/*
  Read PCI config space

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
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>

#include "pcimem.h"

static char *configpath(const char *pcidev) {
	char *path;
	unsigned int domain;
	unsigned int bus;
	unsigned int dev;
	unsigned int func;

	if (sscanf(pcidev, "%d:%x:%x.%d", &domain, &bus, &dev, &func) < 0) {
		fprintf(stderr, "configpath(%s): parse error, %s\n", pcidev, strerror(errno));
		return NULL;
	}

	path = malloc(PATH_MAX);
	if (path == NULL) {
		fprintf(stderr, "out of memory\n");
		return NULL;
	}

	snprintf(path, PATH_MAX, "%s/%2.2x/%2.2x.%d", configdir, bus, dev, func);

	return path;
}

void *readconfig(struct state *state, unsigned int *len) {
	struct stat st;
	char *path = NULL;
	char *p = NULL;
	int fd;
	ssize_t n;

	if ((path = configpath(state->pcidev)) == NULL)
		return NULL;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		printf("open(%s): %s\n", path, strerror(errno));
		goto leave;
	}

	if (fstat(fd, &st)) {
		fprintf(stderr, "fstat(%s): %s\n", path, strerror(errno));
		goto leave;
	}

	p = malloc(st.st_size);

	n = read(fd, p, st.st_size);
	if (n < 0) {
		fprintf(stderr, "read(%s): %s\n", path, strerror(errno));
		free(p);
		p = NULL;
		goto leave;
	} else if (n != st.st_size) {
		fprintf(stderr, "short read(%s) got %d expected %d\n", path, (int)n, (int)st.st_size);
		free(p);
		p = NULL;
		goto leave;
	}

	*len = (unsigned int) n;
leave:
	free(path);

	if (fd >= 0)
		if (close(fd))
			fprintf(stderr, "close: %s\n", strerror(errno));

	return p;
}
