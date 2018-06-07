CFLAGS += -ggdb

all: pcimem

objs :=						\
	pcimem.o				\
	findres.o				\
	openres.o				\

pcimem: $(objs)

clean:
	$(RM) $(objs) pcimem

.PHONY: clean
