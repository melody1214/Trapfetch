all: wrapper tracer test-rdtsc test-sscanf test-clock
CC = gcc

wrapper: wrapper/wrapper.c
	$(CC) $^ -fPIC -shared -ldl -lm -DARCH=64 -o $@.x86_64.so
	$(CC) $^ -fPIC -shared -ldl -lm -m32 -DARCH=32 -o $@.i386.so

tracer: tracer/tracer.h tracer/tracer.c common/hash.c tracer/main.c
	$(CC) $^ -lm -g -DARCH=64 -o $@.x86_64
	$(CC) $^ -lm -g -DARCH=32 -m32 -o $@.i386

test-rdtsc: test/rdtsc.c test/rdtsc.h
	$(CC) test/rdtsc.c -o test/rdtsc.bin

test-sscanf: test/sscanf.c
	$(CC) test/sscanf.c -o test/sscanf.bin

test-clock: test/clock.c
	$(CC) test/clock.c -lrt -o test/clock.bin

clean:
	@find . \( -type f -name '*.x86_64' -or -name '*.*386' -or -name '*.so' -or -name '*.bin' \) -delete
