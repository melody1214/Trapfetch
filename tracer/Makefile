CC = gcc
CFLAGS_64 = -Wall -lm -g -DARCH=64 -DHOME=$(HOME)
CFLAGS_32 = -Wall -lm -g -m32 -DARCH=32 
PROGRAM = tracer
SOURCES = tracer.c ../common/hash.c main.c
INCLUDES = tracer.h

$(PROGRAM) : $(INCLUDES) $(SOURCES)
	$(CC) $^ $(CFLAGS_64) -o $@.x86_64
###	$(CC) $^ $(CFLAGS_32) -o $@.i386

clean:
	rm -f *.x86_64 *.i386
