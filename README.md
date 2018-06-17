A utility for reading/writing PCI MMIO or config space from userspace.

This tool provides a command prompt for the user, allowing selection
of PCI device, PCI BAR or config mode. There is a help command as well
as command completion.

~~~~text
usage: ./pcimem [-p <domain>] -s <bus>:<dev>:<func>
       -h help
       -d <pci domain> [default 0]
       -s <pci device bus:dev:func>
       -b <bar #>
       -r <radix> from [2, 10, 16]
       -x <hex format> from [1]byte, [2]word, [4]double-word
~~~~

One can choose the input radix for arguments - this does not apply to
PCI device names, which are strings that need to match their
associated link name in sysfs.

The hex format setting allows one to choose from byte, word or
doubleword aggregation of data in the hex output the read commands.

Once a device and bar/cfg are selected one can read or write a byte,
word or doubleword. Reading a range is also available.

~~~~text
$ ./pcimem 
nopci> help
help                 display help
pci                  list or change pci device
bar                  choose a PCI BAR
cfg                  use config space
rd                   read range
r1                   read byte
r2                   read word
r4                   read double-word
r8                   read quad-word
w1                   write byte
w2                   write word
w4                   write double-word
radix                set radix
hex                  set hex dump width
~~~~

# Todo

* Subcommand completion to help select PCI or PCI BAR
