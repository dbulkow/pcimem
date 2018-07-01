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
	char	 pcidev[100];	// pci device name
	int	 fd;		// file descriptor of resource file
	int	 res;		// resource number
	void	*map;		// pointer to mapped memory
	int	 maplen;	// size of mapped memory
	int	 cfgfd;		// config file descriptor
	int	 cfglen;	// size of config file
	int	 radix;		// base of input values
	int	 hex_format;	// determines hex display width
};

#define nelem(a) (sizeof(a)/sizeof(a[0]))

int pcidir(char *pcidev, char *abspath);
int closeres(struct state *state);
int opencfg(struct state *state);
int closecfg(struct state *state);

void hexdump(int width, const char *p, const char *ep);
void hexdumpn(int width, const char *p, int n);

int command(struct state *state);

int pcicmd(struct state *state, int argc, char **argv);
int barcmd(struct state *state, int argc, char **argv);
int rwcmd(struct state *state, int argc, char **argv);
