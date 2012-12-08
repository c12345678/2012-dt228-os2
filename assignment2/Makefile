BINS = shm_server shm_client
PWD = $(shell pwd)

default: $(BINS)

shm_server: shm_server.c
	$(CC) -g -O0 -Wall $< -o $@ -lpthread -lrt

shm_client: shm_client.c
	$(CC) -g -O0 -Wall $< -o $@ -lpthread -lrt

.PHONY: clean

tests: $(BINS)
	@tests/test_runner.rb $(PWD)/shm_server $(PWD)/shm_client

grade: $(BINS)
	@tests/test_runner.rb $(PWD)/shm_server $(PWD)/shm_client --grade

clean:
	rm -f *~ *.o $(BINS) *.out
