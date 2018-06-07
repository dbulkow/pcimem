/*
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

extern char *devicedir;

struct state {
	char pcidev[100]; // pci device name
	int fd; // file descriptor of resource file
	int res; // resource number
};

int pcidir(char *pcidev, char *abspath);
int command(char *pcidev, int domain);

int listcmd(struct state *state, int argc, char **argv);
int pcicmd(struct state *state, int argc, char **argv);
int barcmd(struct state *state, int argc, char **argv);
int helpcmd(struct state *state, int argc, char **argv);
int exitcmd(struct state *state, int argc, char **argv);
