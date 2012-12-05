BINS = shm_server shm_client

default: $(BINS)

shm_server: shm_server.c
	$(CC) -g -O0 -Wall -c $< -o $@ -lpthread -lrt

shm_client: shm_client.c
	$(CC) -g -O0 -Wall -c $< -o $@ -lpthread -lrt

.PHONY: clean

tests: $(BINS)
	@echo 1>&2 '***************************************************************'
	@echo 1>&2 'No tests available to run. These will be made available shortly'
	@echo 1>&2 '***************************************************************'

clean:
	rm -f *~ *.o $(BINS) *.out
