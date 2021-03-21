CC = gcc
CFLAGS = -Wall -g -DHOME=$(HOME)
PROGRAM = prefetcher
SOURCES = file.c prefetcher.c main.c
INCLUDES = file.h prefetcher.h

$(PROGRAM) : $(INCLUDES) $(SOURCES)
	$(CC) $^ $(CFLAGS) -o $@.x86_64

clean:
	rm -f *.x86_64