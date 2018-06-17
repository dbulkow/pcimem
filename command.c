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

int cfgcmd(struct state *state, int argc, char **argv) {
	closeres(state);
	state->res = -1;
	return 0;
}

int radixcmd(struct state *state, int argc, char **argv) {
	int radix;

	if (argc < 2) {
		radix = state->radix;
		if (radix == 0)
			radix = 10;
		printf("radix: %d\n", radix);
		return 0;
	}

	radix = atoi(argv[1]);

	switch (radix) {
	case 10:
		radix = 0;
	case 2:
	case 16:
		break;
	default:
		fprintf(stderr, "invalid radix... unchanged\n");
		return 0;
	}

	state->radix = radix;
	return 0;
}

int hexcmd(struct state *state, int argc, char **argv) {
	int hex;

	if (argc < 2) {
		printf("hex format: ");

		switch (state->hex_format) {
		case 1:
			printf("1 - byte\n");
			break;
		case 2:
			printf("2 - word\n");
			break;
		case 4:
			printf("4 - doubleword\n");
			break;
		case 8:
			printf("8 - quadword\n");
			break;
		default:
			printf("default - byte\n");
		}
		return 0;
	}

	hex = atoi(argv[1]);

	switch (hex) {
	case 1:
	case 2:
	case 4:
	case 8:
		break;
	default:
		fprintf(stderr, "invalid hex format\n");
		return 0;
	}

	state->hex_format = hex;
	return 0;
}

static struct cmd {
	char *cmd_name;
	int (*cmd_func)(struct state *state, int argc, char **argv);
	char *cmd_help;
} cmds[] = {
	{"help", helpcmd, "display help"},
	{"exit", exitcmd, ""},
	{"quit", exitcmd, ""},
	{"q", exitcmd, ""},
	{"pci", pcicmd, "list or change pci device"},
	{"bar", barcmd, "choose a PCI BAR"},
	{"cfg", cfgcmd, "use config space"},
	{"rd", rwcmd, "read range"},
	{"r1", rwcmd, "read byte"},
	{"r2", rwcmd, "read word"},
	{"r4", rwcmd, "read double-word"},
	{"r8", rwcmd, "read quad-word"},
	{"w1", rwcmd, "write byte"},
	{"w2", rwcmd, "write word"},
	{"w4", rwcmd, "write double-word"},
	{"radix", radixcmd, "set radix"},
	{"hex", hexcmd, "set hex dump width"},
};

int exitcmd(struct state *unused, int argc, char **argv) {
	return 1;
}

int helpcmd(struct state *unused, int argc, char **argv) {
	int i;

	for (i = 0; i < nelem(cmds); i++)
		if (strlen(cmds[i].cmd_help) > 0)
			printf("%-20s %s\n", cmds[i].cmd_name, cmds[i].cmd_help);

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

static char **command_completion(const char *text, int start, int end) {
	// disable filename completion
	rl_attempted_completion_over = 1;

	if (start == 0) {
		char **names = calloc(nelem(cmds)+2, sizeof(char*));
		int i, j;
		int matches = 0;

		names[0] = strdup(text);
		for (i = 0, j = 1; i < nelem(cmds); i++) {
			if (strncmp(text, cmds[i].cmd_name, strlen(text)) == 0) {
				names[j++] = strdup(cmds[i].cmd_name);
				matches++;
			}
		}

		if (matches == 1) {
			names[0] = names[1];
			names[1] = NULL;
		}

		return names;
	}

	return NULL;
}

int command(struct state *state) {
	char *line = NULL;

	rl_attempted_completion_function = command_completion;

	for (;;) {
		char prompt[80];
		int argc;
		char *argv[80];

		if (line != NULL)
			free(line);

		if (strlen(state->pcidev) == 0)
			strcpy(prompt, "nopci> ");
		else if (state->res < 0)
			snprintf(prompt, sizeof(prompt), "%s[cfg]> ", state->pcidev);
		else
			snprintf(prompt, sizeof(prompt), "%s[bar%d]> ", state->pcidev, state->res);

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

		if (runcmd(state, argc, argv))
			break;
	}

	closeres(state);
	return 0;
}
