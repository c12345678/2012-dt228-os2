BINS = shm_server shm_client

DEPS = Makefile
CFLAGS = -g -O0 -Wall
LDFLAGS = -lpthread -lrt

default: $(BINS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< $(LDFLAGS)

.PHONY: clean

tests: $(BINS)
	@echo 1>&2 '***************************************************************'
	@echo 1>&2 'No tests available to run. These will be made available shortly'
	@echo 1>&2 '***************************************************************'

clean:
	rm -f *~ *.o $(BINS) *.out
