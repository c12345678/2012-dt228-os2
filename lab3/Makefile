CFLAGS=-g -O0 -Wall

default: lsh

OBJS = lsh.o tokenise.o symtab.o
DEPS = tokenise.h symtab.h

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

lsh: $(OBJS)

tokenise:
	$(CC) -DTOKENISE_TEST $(CFLAGS) -o $@ $@.c

.PHONY: clean

clean:
	rm -f lsh *~ *.o tokenise lsh dup2
