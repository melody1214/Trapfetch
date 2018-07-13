all: wrapper tracer
CC = gcc

wrapper: wrapper/wrapper.c
	$(CC) $^ -fPIC -shared -ldl -lm -DARCH=64 -o $@.x86_64.so
	$(CC) $^ -fPIC -shared -ldl -lm -m32 -DARCH=32 -o $@.i386.so

tracer: tracer/tracer.h tracer/tracer.c common/hash.c tracer/main.c
	$(CC) $^ -lm -g -DARCH=64 -o $@.x86_64
	$(CC) $^ -lm -g -DARCH=32 -m32 -o $@.i386

clean:
	@rm -rf *.x86_64 *.i386 *.so
