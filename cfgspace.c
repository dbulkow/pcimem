/*
  Open/close PCI config space

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

int opencfg(struct state *state) {
	struct stat st;
	char path[PATH_MAX];
	int fd;

	snprintf(path, sizeof(path), "%s/%s/config", devicedir, state->pcidev);

	fd = open(path, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "open(%s): %s\n", path, strerror(errno));
		return -1;
	}

	if (fstat(fd, &st) < 0) {
		fprintf(stderr, "fstat(%s): %s\n", path, strerror(errno));
		return -1;
	}

	state->cfglen = st.st_size;

	return fd;
}

int closecfg(struct state *state) {
	state->cfglen = -1;

	if (state->fd >= 0) {
		if (close(state->fd)) {
			fprintf(stderr, "close(%s): %s\n", state->pcidev, strerror(errno));
			return -1;
		}
	}

	state->fd = -1;
	return 0;
}
