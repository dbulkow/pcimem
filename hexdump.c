/*
  Display data in hex

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
#include <ctype.h>

#include "pcimem.h"

void hexdump(int width, const char *p, const char *ep) {
	int i, j;
	int n;
	unsigned char *ap;
	unsigned char old[16];
	int repeat;
	int quiet;
	int first;
	unsigned d;

	ap = (unsigned char *) p;

	quiet = 0;
	first = 1;
	n = ep-p;
	for (i = 0; i < n; i+=16) {
		repeat = 1;
		for (j = 0; i&& j < 16; j++) {
			if (old[j] != ap[i+j]) {
				repeat = 0;
				quiet = 0;
				break;
			}
		}
		if (!first && repeat) {
			if (!quiet) {
				printf("*\n");
				quiet = 1;
			}
			continue;
		}
		first = 0;

		if (!quiet)
			printf("%8.8x  ", i);

		for (j = 0; j < 16; j++) {
			old[j] = ap[i+j];
			if (j == 8)
				printf(" ");
			if (i+j < n) {
				unsigned int *lp;

				if ((j % width) == 0) {
					switch (width) {
					case 1:
						printf("%2.2x ", ap[i+j]);
						break;
					case 2:
						printf("%2.2x%2.2x ", ap[i+j+1], ap[i+j]);
						break;
					case 4:
						lp = (unsigned int *) &ap[i+j];
						printf("%8.8x ", *lp);
						break;
					case 8:
						lp = (unsigned int *) &ap[i+j+4];
						printf("%8.8x", *lp);
						lp = (unsigned int *) &ap[i+j];
						printf("%8.8x ", *lp);
						break;
					}
				}
			} else if ((j % width) == 0) {
				int k;
				for (k = 0; k < width*2; k++)
					printf(" ");
				printf(" ");
			}
		}

		printf(" |");
		for (j = 0; j < 16; j++)
			if (i+j < n)
				printf("%c", isprint(ap[i+j])?ap[i+j]:'.');
		printf("|\n");
	}
	d = ep - p;
	printf("%8.8x\n", d);
}

void hexdumpn(int width, const char *p, int n) {
	hexdump(width, p, p+n);
}
