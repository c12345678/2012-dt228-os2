BINS = shared_counter_doesnt_work \
			 shared_counter_works_1 \
			 shared_counter_works_2 \
			 shared_counter_works_3

DEPS = Makefile
CFLAGS = -g -O0 -Wall
LDFLAGS = -lpthread

default: $(BINS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< $(LDFLAGS)

.PHONY: clean

clean:
	rm -f *~ *.o $(BINS) *.out
