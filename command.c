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
#include <readline/history.h>

#include "pcimem.h"

int getargs(char *text, char **args, int maxarg) {
	char *tok;
	int count;

	count = 0;
	tok = strtok(text, " ");
	while (tok != NULL) {
		if (count >= maxarg)
			break;
		args[count++] = tok;
		tok = strtok(NULL, " ");
	}

	return count;
}

#define nelem(a) (sizeof(a)/sizeof(a[0]))

struct cmd {
	char *cmd_name;
	int (*cmd_func)(struct state *state, int argc, char **argv);
	char *cmd_help;
	char *cmd_longhelp;
} cmds[] = {
	{"help", helpcmd, "help", "display help"},
	{"exit", exitcmd, "", ""},
	{"quit", exitcmd, "", ""},
	{"q", exitcmd, "", ""},
	{"pci", pcicmd, "pci [device]", "list or change pci device"},
	{"bar", barcmd, "bar [bar number]", "choose a PCI BAR"},
};

int exitcmd(struct state *unused, int argc, char **argv) {
	return 1;
}

int helpcmd(struct state *unused, int argc, char **argv) {
	int i;

	for (i = 0; i < nelem(cmds); i++) {
		if (strlen(cmds[i].cmd_help) > 0) {
			printf("%-20s %s\n", cmds[i].cmd_help, cmds[i].cmd_longhelp);
		}
	}

	return 0;
}

int runcmd(struct state *state, int argc, char **argv) {
	int i;

	for (i = 0; i < nelem(cmds); i++) {
		if (strcmp(cmds[i].cmd_name, argv[0]) == 0) {
			return cmds[i].cmd_func(state, argc, argv);
		}
	}

	printf("unknown command: %s\n", argv[0]);

	return 0;
}

int command(char *pcidev, int domain) {
	struct state state;
	char *line = NULL;

	state.fd = -1;
	state.res = -1;
	state.map = NULL;
	state.maplen = -1;

	if (pcidev == NULL)
		state.pcidev[0] = '\0';
	else
		snprintf(state.pcidev, sizeof(state.pcidev), "%04d:%s", domain, pcidev);

	rl_bind_key('\t', rl_abort);

	for (;;) {
		char prompt[80];
		int argc;
		char *argv[80];

		if (line != NULL)
			free(line);

		if (strlen(state.pcidev) == 0)
			strcpy(prompt, "nopci> ");
		else if (state.res < 0)
			snprintf(prompt, sizeof(prompt), "%s> ", state.pcidev);
		else
			snprintf(prompt, sizeof(prompt), "%s[bar%d]> ", state.pcidev, state.res);

		line = readline(prompt);
		if (line == NULL) {
			printf("\n");
			break;
		}

		if (line[0] == '\0')
			continue;

		add_history(line);

		if ((argc = getargs(line, argv, nelem(argv))) == 0)
			continue;

		switch (runcmd(&state, argc, argv)) {
		case 0:
			continue;
		default:
			goto done;
		}
	}
done:
	closeres(&state);
	return 0;
}
