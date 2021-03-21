.PHONY: tracer analyzer wrapper prefetcher test clean
all: tracer analyzer wrapper prefetcher test

tracer:
	cd tracer; make

analyzer:
	cd analyzer; make

prefetcher:
	cd prefetcher; make

wrapper:
	cd wrapper; make

test:
	cd test; make

clean:
	cd tracer; make clean
	cd analyzer; make clean
	cd prefetcher; make clean
	cd wrapper; make clean
	cd test; make clean