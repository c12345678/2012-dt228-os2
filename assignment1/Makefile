BIN=lsh

default: $(BIN)

OBJS = lsh.o tokenise.o symtab.o internal.o execute.o
DEPS = tokenise.h symtab.h internal.h Makefile tests/test_runner.rb

FEATURES = \
	   -DLSH_ENABLE_CD \
	   -DLSH_ENABLE_ENV \
	   -DLSH_ENABLE_EXTERNAL \
	   -DLSH_ENABLE_USERVARS

PROMPT = ">> "

CFLAGS=-g -O0 -Wall $(FEATURES) -DPS1='$(PROMPT)'

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

lsh: $(OBJS)

tokenise: tokenise.o
	$(CC) -DTOKENISE_TEST $(CFLAGS) -o $@ $@.c

symtab: symtab.o
	$(CC) -DSYMTAB_TEST $(CFLAGS) -o $@ $@.c


.PHONY: clean listfeatures showprompt

tests: lsh
	tests/test_runner.rb $(shell pwd)/$(BIN)

grade: lsh
	@tests/test_runner.rb $(shell pwd)/$(BIN) --grade

clean:
	rm -f lsh *~ *.o tokenise lsh symtab tests/*~

listfeatures:
	@echo $(FEATURES)

showprompt:
	@echo $(PROMPT)
