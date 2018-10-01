#!/bin/sh

APPNAME=$1

if [ $APPNAME = "speed-dreams-2" ] || [ $APPNAME = "fgfs" ] || [ $APPNAME = "vegastrike" ] || [ $APPNAME = "eclipse" ]
then
	if [ $APPNAME = "speed-dreams-2" ]
	then
		LD_PRELOAD=../bin/libwrapper.so ../bin/gatherer /usr/games/speed-dreams-2
	elif [ $APPNAME = "fgfs" ]
	then
		LD_PRELOAD=../bin/libwrapper.so ../bin/gatherer /usr/games/fgfs
	elif [ $APPNAME = "vegastrike" ]
	then
		LD_PRELOAD=../bin/libwrapper.so ../bin/gatherer /usr/games/vegastrike
	elif [ $APPNAME = "eclipse" ]
	then
		LD_PRELOAD=../bin/libwrapper.so ../bin/gatherer /home/melody/eclipse/java-neon/eclipse/eclipse
	fi
else
	echo "usage: ./gatherer.sh <appname>"
fi

