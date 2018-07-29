all: wrapper tracer analyzer test-rdtsc test-sscanf test-clock
CC = gcc

wrapper: wrapper/wrapper.c
	$(CC) $^ -Wall -fPIC -shared -ldl -lm -DARCH=64 -o $@.x86_64.so
	$(CC) $^ -Wall -fPIC -shared -ldl -lm -m32 -DARCH=32 -o $@.i386.so

tracer: tracer/main.c tracer/tracer.h tracer/tracer.c common/hash.c
	$(CC) $^ -Wall -lm -g -DARCH=64 -o $@.x86_64
	$(CC) $^ -Wall -lm -g -DARCH=32 -m32 -o $@.i386

analyzer: analyzer/analyzer.h analyzer/analyzer.c analyzer/file.c analyzer/queue.h analyzer/queue.c common/hash.c analyzer/main.c
	$(CC) $^ -Wall -g -o $@.x86_64

test-rdtsc: test/rdtsc.c test/rdtsc.h
	$(CC) test/rdtsc.c -o test/rdtsc.bin

test-sscanf: test/sscanf.c
	$(CC) test/sscanf.c -o test/sscanf.bin

test-clock: test/clock.c
	$(CC) test/clock.c -lrt -o test/clock.bin

clean:
	@find . \( -type f -name '*.x86_64' -or -name '*.*386' -or -name '*.so' -or -name '*.bin' \) -delete
