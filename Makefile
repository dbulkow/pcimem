CFLAGS += -ggdb -Wall -Werror
LDLIBS += -lreadline

all: pcimem

objs :=						\
	pcimem.o				\
	command.o				\
	barcmd.o				\
	pcicmd.o				\

$(objs): pcimem.h

pcimem: $(objs)

clean:
	$(RM) $(objs) pcimem

.PHONY: clean
