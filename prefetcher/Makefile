DEVNAME = $(shell lsblk -oMOUNTPOINT,PKNAME -P -M | grep 'MOUNTPOINT="/"' | sed -e 's;^.*PKNAME=;;' | sed 's/"//g')
ROTATIONAL = $(shell cat /sys/block/$(DEVNAME)/queue/rotational)
DISKTYPE =

ifeq ($(ROTATIONAL), 0)
	DISKTYPE = SSD
else
	DISKTYPE = HDD
endif

CC = gcc
CFLAGS = -Wall -lm -g -DARCH=64 -D$(DISKTYPE) -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -lpthread -DHOME=$(HOME)
PROGRAM = prefetcher
SOURCES = file.c ../common/hash.c prefetcher.c main.c
INCLUDES = file.h prefetcher.h

$(PROGRAM) : $(INCLUDES) $(SOURCES)
	$(CC) $^ $(CFLAGS) -o $@.x86_64

clean:
	rm -f *.x86_64
