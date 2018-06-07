/*
  Command processing

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
#include <readline/readline.h>

int command(char *pcidev, int res) {
	char prompt[80];
	char *line;
	int fd;

	snprintf(prompt, sizeof(prompt), "%s[bar%d]> ", pcidev, res);

	fd = openres(pcidev, res);

	while ((line = readline(prompt)) != NULL) {
		printf("%s\n", line);
	}

	closeres(fd);

	printf("\n");
}
