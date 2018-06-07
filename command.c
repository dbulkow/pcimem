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
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>

int getargs(char *text, char **args, int maxarg) {
	char *tok;
	char *arg;
	char *cmd;
	int count, index;

	count = 0;
	tok = strtok(text, " ");
	while (tok != NULL) {
		args[count++] = tok;
		tok = strtok(NULL, " ");
	}

	return count;
}

int iscmd(char *cmd, char *text) {
	if (strcmp(cmd, text) == 0)
		return 1;
	return 0;
}

#define nelem(a) (sizeof(a)/sizeof(a[0]))

int command(char *pcidev, int res) {
	char prompt[80];
	char *line;
	int fd;

	snprintf(prompt, sizeof(prompt), "%s[bar%d]> ", pcidev, res);

	fd = openres(pcidev, res);

	for (;;) {
		int argc;
		char *argv[80];
		char *cmd;

		line = readline(prompt);
		if (line == NULL) {
			printf("\n");
			break;
		}

		argc = getargs(line, argv, nelem(argv));

		cmd = argv[0];

		if (iscmd(cmd, "list")) {
			findres(pcidev);
		} else if (iscmd(cmd, "exit")) {
			break;
		} else if (iscmd(cmd, "quit")) {
			break;
		}
	}

	closeres(fd);
}
