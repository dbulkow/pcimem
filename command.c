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

static int getargs(char *text, char **args, int maxarg) {
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

static int cfgcmd(struct state *state, int argc, char **argv) {
	closeres(state);
	state->res = -1;
	return 0;
}

static int radixcmd(struct state *state, int argc, char **argv) {
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

static char *radix_complete(const char *text, int state) {
	static char *fmts[] = {"2", "10", "16", NULL};
	static int index, len;
	char *name;

	if (state == 0) {
		index = 0;
		len = strlen(text);
	}

	while ((name = fmts[index++]))
		if (strncmp(name, text, len) == 0)
			return strdup(name);

	return NULL;
}

static int hexcmd(struct state *state, int argc, char **argv) {
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

static char *hex_complete(const char *text, int state) {
	static char *fmts[] = {"1", "2", "4", "8", NULL};
	static int index, len;
	char *name;

	if (state == 0) {
		index = 0;
		len = strlen(text);
	}

	while ((name = fmts[index++]))
		if (strncmp(name, text, len) == 0)
			return strdup(name);

	return NULL;
}

static int exitcmd(struct state *unused, int argc, char **argv) {
	return 1;
}

static int helpcmd(struct state *unused, int argc, char **argv);

static struct cmd {
	char *cmd_name;
	int (*cmd_func)(struct state *state, int argc, char **argv);
	char *(*cmd_completion)(const char *text, int state);
	char *cmd_help;
} cmds[] = {
	{"help",  helpcmd,  NULL, "display help"},
	{"exit",  exitcmd,  NULL, ""},
	{"quit",  exitcmd,  NULL, ""},
	{"q",     exitcmd,  NULL, ""},
	{"pci",   pcicmd,   pci_complete, "list or change pci device"},
	{"bar",   barcmd,   NULL, "choose a PCI BAR"},
	{"cfg",   cfgcmd,   NULL, "use config space"},
	{"rd",    rwcmd,    NULL, "read range"},
	{"r1",    rwcmd,    NULL, "read byte"},
	{"r2",    rwcmd,    NULL, "read word"},
	{"r4",    rwcmd,    NULL, "read double-word"},
	{"r8",    rwcmd,    NULL, "read quad-word"},
	{"w1",    rwcmd,    NULL, "write byte"},
	{"w2",    rwcmd,    NULL, "write word"},
	{"w4",    rwcmd,    NULL, "write double-word"},
	{"radix", radixcmd, radix_complete, "set radix"},
	{"hex",   hexcmd,   hex_complete, "set hex dump width"},
};

static int helpcmd(struct state *unused, int argc, char **argv) {
	int i;

	for (i = 0; i < nelem(cmds); i++)
		if (strlen(cmds[i].cmd_help) > 0)
			printf("%-20s %s\n", cmds[i].cmd_name, cmds[i].cmd_help);

	return 0;
}

static int runcmd(struct state *state, int argc, char **argv) {
	int i;

	for (i = 0; i < nelem(cmds); i++) {
		if (strcmp(cmds[i].cmd_name, argv[0]) == 0) {
			return cmds[i].cmd_func(state, argc, argv);
		}
	}

	printf("unknown command: %s\n", argv[0]);

	return 0;
}

static char *cmds_completion(const char *text, int state) {
	static int index, len;
	char *name;

	if (state == 0) {
		index = 0;
		len = strlen(text);
	}

	while ((name = cmds[index++].cmd_name))
		if (strncmp(name, text, len) == 0)
			return strdup(name);

	return NULL;
}

static char **command_completion(const char *text, int start, int end) {
	rl_compentry_func_t *completion;
	char **names = NULL;
	char *buf;
	char *args[3];
	int n;
	int index = -1;
	int i;

	// disable filename completion
	rl_attempted_completion_over = 1;

	if (start == 0)
		return rl_completion_matches(text, cmds_completion);

	buf = strdup(rl_line_buffer);
	n = getargs(buf, args, nelem(args));

	// get out if already processed command argument
	if (n >= 2 && rl_line_buffer[strlen(rl_line_buffer)-1] == ' ')
		goto bail;

	for (i = 0; i < nelem(cmds); i++)
		if (strcmp(cmds[i].cmd_name, args[0]) == 0)
			index = i;

	if (index == -1) {
		fprintf(stderr, "\nunknown command: %s\n", args[0]);
		goto bail;
	}

	completion = cmds[index].cmd_completion;
	if (completion == NULL)
		goto bail;

	names = rl_completion_matches(text, completion);
bail:
	free(buf);
	return names;
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
