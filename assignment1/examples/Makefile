BINS = redirectto redirectfrom pipeline pipeline2 background
default: $(BINS)

OBJS = redirectto.o redirectfrom.o pipeline.o pipeline2.o background.o
DEPS = Makefile 

CFLAGS=-g -O0 -Wall

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $<

.PHONY: clean

clean:
	rm -f lsh *~ *.o $(BINS) *.txt
