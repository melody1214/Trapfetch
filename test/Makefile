CC = gcc

all: test-rdtsc test-sscanf test-clock test-hash

test-rdtsc: rdtsc.c rdtsc.h
	$(CC) rdtsc.c -g -o rdtsc.x86_64

test-sscanf: sscanf.c
	$(CC) sscanf.c -g -o sscanf.x86_64

test-clock: clock.c
	$(CC) clock.c -g -lrt -o clock.x86_64

test-hash: hash.c
	$(CC) hash.c -g -o hash.x86_64

test-mkpath: mkpath.c
	$(CC) mkpath.c -g -o mkpath.x86_64

test-address-sub: address_sub.c
	$(CC) address_sub.c -g -o address_sub.x86_64

test-tracing: tracing.c
	$(CC) tracing.c -g -o tracing.x86_64

test-threads: threads.c
	$(CC) threads.c -g -o threads.x86_64


clean:
	rm -f *.x86_64