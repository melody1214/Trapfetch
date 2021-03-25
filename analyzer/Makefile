CC = gcc
CFLAGS = -DHOME=$(HOME) -Wall -g
PROGRAM = analyzer
SOURCES = ../common/hash.c analyzer.c file.c queue.c main.c
INCLUDES = analyzer.h file.h queue.h

$(PROGRAM) : $(INCLUDES) $(SOURCES)
	$(CC) $^ $(CFLAGS) -o $@.x86_64

clean:
	rm -f *.x86_64:Wall