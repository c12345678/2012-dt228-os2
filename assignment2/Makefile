BINS = shm_server shm_client

DEPS = Makefile
CFLAGS = -g -O0 -Wall
LDFLAGS = -lpthread -lrt

default: $(BINS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f *~ *.o $(BINS) *.out
