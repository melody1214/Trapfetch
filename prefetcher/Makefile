CC = gcc
CFLAGS = -Wall -g -DHOME=$(HOME) -DARCH=64 -DSSD -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -pthread
PROGRAM = prefetcher
SOURCES = file.c prefetcher.c main.c ../common/hash.c
INCLUDES = file.h prefetcher.h

$(PROGRAM) : $(INCLUDES) $(SOURCES)
	$(CC) $^ $(CFLAGS) -o $@.x86_64

clean:
	rm -f *.x86_64