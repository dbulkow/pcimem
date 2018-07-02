/*
  PCI device lookup and assignment

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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#include <limits.h>
#include <errno.h>
#include <pci/pci.h>

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

static struct pci_dev *lookup_pci(struct pci_access *pacc, const char *pcidev) {
	struct pci_dev *dev;
	int domain, bus, device, func;
	int n;

	n = sscanf(pcidev, "%x:%x:%x.%d", &domain, &bus, &device, &func);
	if (n != 4) {
		fprintf(stderr, "device scan error: %s\n", strerror(errno));
		return NULL;
	}

	for (dev = pacc->devices; dev; dev = dev->next)
		if (dev->domain == domain && dev->bus == bus && dev->dev == device && dev->func == func)
			return dev;

	return NULL;
}

static int listpci(void) {
	DIR *dir;
	struct dirent *dent;
	struct pci_access *pacc;
	struct pci_dev *dev;
	char namebuf[1024], *name;
	char classbuf[128], *class;

	pacc = pci_alloc();
	pci_init(pacc);
	pci_scan_bus(pacc);

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

		dev = lookup_pci(pacc, dent->d_name);

		if (dev == NULL) {
			printf("%s\n", dent->d_name);
			continue;
		}

		pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);

		name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
		class = pci_lookup_name(pacc, classbuf, sizeof(classbuf), PCI_LOOKUP_CLASS, dev->device_class);

		printf("%04x:%02x:%02x.%d %s: %s\n", dev->domain, dev->bus, dev->dev, dev->func, class, name);
	}

	closedir(dir);

	pci_cleanup(pacc);

	return 0;
}

static int setpci(struct state *state, char *pcidev) {
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
	state->res = -1;

	closecfg(state);
	if ((state->cfgfd = opencfg(state)) == -1)
		return 0;

	return 0;
}

int pcicmd(struct state *state, int argc, char **argv) {
	if (argc > 1)
		return setpci(state, argv[1]);

	return listpci();
}

char *pci_complete(const char *text, int state) {
	static char **devs;
	static int index, len;
	char *name;

	if (state == 0) {
		DIR *dir;
		struct dirent *dent;
		int i;

		index = 0;
		len = strlen(text);

		devs = calloc(1, sizeof(char *));

		dir = opendir(devicedir);
		if (dir == NULL)
			return NULL;

		i = 0;
		while ((dent = readdir(dir)) != NULL) {
			if (strcmp(dent->d_name, ".") == 0 ||
			    strcmp(dent->d_name, "..") == 0) {
				continue;
			}

			devs[i++] = strdup(dent->d_name);

			devs = realloc(devs, sizeof(char *) * (i+1));
			devs[i] = NULL;
		}

		closedir(dir);
	}

	while ((name = devs[index++]))
		if (strncmp(name, text, len) == 0)
			return strdup(name);

	return NULL;
}
