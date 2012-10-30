BINS = redirectto redirectfrom pipeline pipeline2
default: $(BINS)

OBJS = redirectto.o redirectfrom.o pipeline.o pipeline2
DEPS = Makefile 

CFLAGS=-g -O0 -Wall

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	rm -f lsh *~ *.o $(BINS) *.txt
