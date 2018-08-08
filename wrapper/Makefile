CC = gcc
CFLAGS_64 = -Wall -fPIC -shared -ldl -lm -g -DARCH=64
CFLAGS_32 = -Wall -fPIC -shared -ldl -lm -g -m32 -DARCH=32 
PROGRAM = wrapper
SOURCES = wrapper.c

$(PROGRAM) : $(INCLUDES) $(SOURCES)
	$(CC) $^ $(CFLAGS_64) -o $@.x86_64.so
	$(CC) $^ $(CFLAGS_32) -o $@.i386.so

clean:
	rm -f *.so