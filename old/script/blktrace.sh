#!/bin/sh

APPNAME=$1

if [ $APPNAME = "speed-dreams-2" ] || [ $APPNAME = "fgfs" ] || [ $APPNAME = "vegastrike" ] || [ $APPNAME = "eclipse" ]
then
	if [ $APPNAME = "speed-dreams-2" ]
	then
		if [ $2 = "pf" ]
		then
			blktrace -w 15 -d /dev/$3 -o ../blkout & ../bin/prefetch /usr/games/speed-dreams-2
		elif [ $2 = "normal" ]
		then
			blktrace -d /dev/$3 -w 15 -o ../blkout & /usr/games/speed-dreams-2
		fi
	elif [ $APPNAME = "fgfs" ]
	then
		if [ $2 = "pf" ]
		then
			blktrace -d /dev/$3 -w 30 -o ../blkout & ../bin/prefetch /usr/games/fgfs
		elif [ $2 = "normal" ]
		then
			blktrace -d /dev/$3 -w 30 -o ../blkout & /usr/games/fgfs
		fi
	elif [ $APPNAME = "vegastrike" ]
	then
		if [ $2 = "pf" ]
		then
			blktrace -d /dev/$3 -w 60 -o ../blkout & ../bin/prefetch /usr/games/vegastrike
		elif [ $2 = "normal" ]
		then
			blktrace -d /dev/$3 -w 60 -o ../blkout & /usr/games/vegastrike
		fi
	elif [ $APPNAME = "eclipse" ]
	then
		if [ $2 = "pf" ]
		then
			blktrace -d /dev/$3 -w 20 -o ../blkout & ../bin/prefetch /home/melody/eclipse/java-neon/eclipse/eclipse
		elif [ $2 = "normal" ]
		then
			blktrace -d /dev/$3 -w 20 -o ../blkout & /home/melody/eclipse/java-neon/eclipse/eclipse
		fi
	fi
else
	echo "usage : ./blktrace.sh <appname> <pf or normal> <sda or sdb>"
fi

