CC = gcc

all: test-rdtsc test-sscanf test-clock test-hash

test-rdtsc: rdtsc.c rdtsc.h
	$(CC) rdtsc.c -o rdtsc.x86_64

test-sscanf: sscanf.c
	$(CC) sscanf.c -o sscanf.x86_64

test-clock: clock.c
	$(CC) clock.c -lrt -o clock.x86_64

test-hash: hash.c
	$(CC) hash.c -o hash.x86_64

mkpath: mkpath.c
	$(CC) mkpath.c -o mkpath.x86_64

clean:
	rm -f *.x86_64