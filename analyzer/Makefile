DEVNAME = $(shell lsblk -oMOUNTPOINT,PKNAME -P -M | grep 'MOUNTPOINT="/"' | sed -e 's;^.*PKNAME=;;' | sed 's/"//g')
ROTATIONAL = $(shell cat /sys/block/$(DEVNAME)/queue/rotational)
DISKTYPE =

ifeq ($(ROTATIONAL), 0)
        DISKTYPE = SSD
else
        DISKTYPE = HDD
endif

CC = gcc
CFLAGS = -DHOME=$(HOME) -Wall -g -D$(DISKTYPE) -DARCH=64 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
PROGRAM = analyzer
SOURCES = ../common/hash.c analyzer.c file.c queue.c main.c
INCLUDES = hashmap.h analyzer.h file.h queue.h

$(PROGRAM) : $(INCLUDES) $(SOURCES)
	$(CC) $^ $(CFLAGS) -o $@.x86_64

clean:
	rm -f *.x86_64:Wall
