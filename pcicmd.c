/*
  PCI device looking and assignment

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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <regex.h>
#include <limits.h>
#include <errno.h>

#include "pcimem.h"

char *devicedir = "/sys/bus/pci/devices";

int pcidir(char *pcidev, char *abspath) {
	char path[PATH_MAX];
	char linkpath[PATH_MAX];
	char fullpath[PATH_MAX];
	int n;

	snprintf(path, sizeof(path), "%s/%s", devicedir, pcidev);

	n = readlink(path, linkpath, sizeof(linkpath));
	if (n == -1) {
		printf("readlink(%s): %s\n", path, strerror(errno));
		return 1;
	}

	linkpath[n] = '\0';

	memset(fullpath, 0, sizeof(fullpath));
	strncat(fullpath, devicedir, sizeof(fullpath));
	strncat(fullpath, linkpath, strlen(fullpath)-sizeof(fullpath));

	realpath(fullpath, abspath);

	return 0;
}

int listpci() {
	DIR *dir;
	struct dirent *dent;

	dir = opendir(devicedir);
	if (dir == NULL) {
		printf("opendir(%s): %s\n", devicedir, strerror(errno));
		return 0;
	}

	while ((dent = readdir(dir)) != NULL) {
		if (strcmp(dent->d_name, ".") == 0 ||
		    strcmp(dent->d_name, "..") == 0) {
			continue;
		}
		printf("%s\n", dent->d_name);
	}

	closedir(dir);

	return 0;
}

int setpci(struct state *state, char *pcidev) {
	char newdev[sizeof(state->pcidev)];
	regex_t regex;
	int res;

	res = regcomp(&regex, "^[0-9]{4}:[0-9a-z]{2}:[0-9a-z]{2}\\.[0-9]$", REG_EXTENDED);
	if (res) {
		if (res == REG_ESPACE)
			fprintf(stderr, "regcomp: %s\n", strerror(ENOMEM));
		else
			fprintf(stderr, "regcomp: syntax error in pci device regular expression\n");

		return 1;
	}

	res = regexec(&regex, pcidev, 0, NULL, 0);
	if (res == 0) {
		strncpy(newdev, pcidev, sizeof(newdev));
	} else if (res == REG_NOMATCH) {
		strcpy(newdev, "0000:");
		strncat(newdev, pcidev, sizeof(newdev)-5);
	} else {
		size_t len = regerror(res, &regex, NULL, 0);
		{
			char buf[len];
			(void) regerror(res, &regex, buf, len);
			fprintf(stderr, "regex match failed: %s\n", buf);
		}
		return 0;
	}

	{
		char path[PATH_MAX];
		struct stat st;

		snprintf(path, sizeof(path), "%s/%s", devicedir, newdev);

		if (stat(path, &st) < 0) {
			printf("stat(%s): %s\n", path, strerror(errno));
			return 0;
		}
	}

	closeres(state);
	strncpy(state->pcidev, newdev, sizeof(state->pcidev));

	return 0;
}

int pcicmd(struct state *state, int argc, char **argv) {
	if (argc == 1)
		return listpci();

	if (argc > 1)
		return setpci(state, argv[1]);
}
