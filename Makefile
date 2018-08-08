.PHONY: tracer analyzer wrapper test clean
all: tracer analyzer wrapper test

tracer:
	cd tracer; make

analyzer:
	cd analyzer; make

wrapper:
	cd wrapper; make

test:
	cd test; make

clean:
	cd tracer; make clean
	cd analyzer; make clean
	cd wrapper; make clean
	cd test; make clean