all: wrapper_64 tracer_64 wrapper_32 tracer_32
CC = gcc

wrapper_64: wrapper/wrapper_64.c
	$(CC) $^ -fPIC -shared -ldl -lm -DARCH=64 -o $@.so

wrapper_32: wrapper/wrapper.c
	$(CC) $^ -fPIC -shared -ldl -lm -m32 -DARCH=32 -o $@.so

tracer_64: tracer/tracer.h tracer/tracer.c tracer/main.c
	$(CC) $^ -lm -g -DARCH=64 -o $@.x86_64

tracer_32: tracer/tracer.h tracer/tracer.c tracer/main.c
	$(CC) $^ -lm -g -DARCH=32 -m32 -o $@.i386

clean:
	@rm -rf *.x86_64 *.i386 *.so
