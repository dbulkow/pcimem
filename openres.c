/*
  Open a resource file

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
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

#include "pcimem.h"

int openres(char *pcidev, int res) {
	char abspath[PATH_MAX];
	char respath[PATH_MAX];
	int fd;
//	FILE *f;
//	int i;

	pcidir(pcidev, abspath);
/*
	memset(respath, 0, sizeof(respath));
	strncat(respath, abspath, sizeof(respath));
	strncat(respath, "/resource", strlen(respath)-sizeof(respath));

	f = fopen(respath, "r");
	if (f == NULL) {
		perror("fopen");
		exit(1);
	}

	i = 0;
	while (!feof(f)) {
		unsigned long long start, end, flags;

		fscanf(f, "%llx %llx %llx", &start, &end, &flags);

		if (start > 0)
			printf("resource%d %llx %llx %llx\n", i, start, end, flags);

		i++;
	}

	fclose(f);
*/
	snprintf(respath, sizeof(respath), "%s/resource%d", abspath, res);

	printf("%s\n", respath);

	fd = open(respath, O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	return fd;
}

int closeres(int fd) {
	return close(fd);
}
