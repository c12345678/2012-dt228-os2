BIN=lsh
CFLAGS=-g -O0 -Wall

default: $(BIN)

OBJS = lsh.o tokenise.o symtab.o
DEPS = tokenise.h symtab.h

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

lsh: $(OBJS)

tokenise: tokenise.o
	$(CC) -DTOKENISE_TEST $(CFLAGS) -o $@ $@.c

symtab: symtab.o
	$(CC) -DSYMTAB_TEST $(CFLAGS) -o $@ $@.c


.PHONY: clean

tests: lsh tests/test_runner.rb
	tests/test_runner.rb $(shell pwd)/$(BIN)

clean:
	rm -f lsh *~ *.o tokenise lsh symtab tests/*~
