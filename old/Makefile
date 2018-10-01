all: wrapper caller caller_32 gatherer wrapper_32 gatherer_32 coldstart warmstart analyzer analyzer_32
	
wrapper: sources/libwrapper/wrapper.c
	gcc -fPIC -shared -o bin/libwrapper.so sources/libwrapper/wrapper.c -ldl -lm -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE

#wrapper: sources/libwrapper/libwrap_reads.c sources/libwrapper/libwrap_struct.h
#	gcc -Wall -fPIC -shared -lm -ldl -pthread -o bin/libwrapper.so sources/libwrapper/libwrap_reads.c

wrapper_32: sources/libwrapper/wrapper_32.c
	gcc -fPIC -shared -o bin/libwrapper_32.so sources/libwrapper/wrapper_32.c -m32 -ldl -lm -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE

caller_32: sources/prefetcher/caller_64.c header/structures.h
	gcc -o bin/caller_32 sources/prefetcher/caller_64.c -g -DARCH=32 -m32 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -pthread

caller: sources/prefetcher/caller_64.c header/structures.h
	gcc -o bin/caller_64 sources/prefetcher/caller_64.c -g -DARCH=64 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -pthread

analyzer: sources/analyzer/main.c sources/analyzer/analyzer.h sources/analyzer/pf.h
	gcc -o bin/analyzer sources/analyzer/main.c -g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE

analyzer_32: sources/analyzer_32/main.c sources/analyzer_32/analyzer.h
	gcc -o bin/analyzer_32 sources/analyzer_32/main.c -m32 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE

example: examples/dlopener.c examples/dlreader_1.c examples/dlreader_2.c
	gcc -o /home/shared/prefetch/dlopener examples/dlopener.c -ldl -g
	gcc -o /home/shared/prefetch/dlreader_1.so examples/dlreader_1.c -fPIC -shared -ldl -g
	gcc -o /home/shared/prefetch/dlreader_2.so examples/dlreader_2.c -fPIC -shared -ldl -g

coldstart: sources/coldstart/coldstart.c
	gcc -o bin/coldstart sources/coldstart/coldstart.c -g

warmstart: sources/warmstart/warmstart.c
	gcc -o bin/warmstart sources/warmstart/warmstart.c -g

gatherer: sources/gatherer/*
	gcc -o bin/gatherer sources/gatherer/gatherer.c sources/gatherer/main.c sources/gatherer/stack.c -lm -g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE

gatherer_32: sources/gatherer_32/*
	gcc -o bin/gatherer_32 sources/gatherer_32/gatherer.c sources/gatherer_32/main.c sources/gatherer_32/stack.c -lm -m32 -g -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE

clean: 
	rm bin/*.so
	rm bin/caller
	rm bin/gatherer
	rm bin/prefetch
