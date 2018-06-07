/*
  Find PCI resources in sysfs

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
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>

#include "pcimem.h"

char *pcidir(char *pcidev, char *abspath) {
	char *devicedir = "/sys/bus/pci/devices/";
	char path[PATH_MAX];
	char linkpath[PATH_MAX];
	char fullpath[PATH_MAX];
	int n;

	memset(path, 0, sizeof(path));
	strncat(path, devicedir, sizeof(path));
	strncat(path, pcidev, sizeof(path)-strlen(path));

	n = readlink(path, linkpath, sizeof(linkpath));
	if (n == -1) {
		perror("readlink");
		exit(1);
	}

	linkpath[n] = '\0';

	memset(fullpath, 0, sizeof(fullpath));
	strncat(fullpath, devicedir, sizeof(fullpath));
	strncat(fullpath, linkpath, strlen(fullpath)-sizeof(fullpath));

	realpath(fullpath, abspath);

	return abspath;
}

int findres(char *pcidev) {
	DIR *dir;
	struct dirent *dent;
	char abspath[PATH_MAX];
	int fd;

	pcidir(pcidev, abspath);

	dir = opendir(abspath);
	if (dir == NULL) {
		perror("opendir");
		exit(1);
	}

	while ((dent = readdir(dir)) != NULL) {
/*
		if (strcmp(dent->d_name, "resource") == 0) {
			printf("addrs in %s\n", dent->d_name);
			continue;
		}
*/
		if (strstr(dent->d_name, "resource") == dent->d_name && strlen(dent->d_name) > strlen("resource")) {
			printf("%s\n", dent->d_name);
		}
	}

	closedir(dir);
}
