CFLAGS += -ggdb -Wall -Werror
LDLIBS += -lreadline -lpci

all: pcimem

objs :=						\
	pcimem.o				\
	command.o				\
	barcmd.o				\
	cfgspace.o				\
	hexdump.o				\
	pcicmd.o				\
	rwcmd.o					\

$(objs): pcimem.h

pcimem: $(objs)

clean:
	$(RM) $(objs) pcimem

.PHONY: clean
