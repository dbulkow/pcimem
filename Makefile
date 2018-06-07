CFLAGS += -ggdb
LDFLAGS += -lreadline

all: pcimem

objs :=						\
	pcimem.o				\
	command.o				\
	findres.o				\
	openres.o				\

pcimem: $(objs)

clean:
	$(RM) $(objs) pcimem

.PHONY: clean
