#!/bin/sh

APPNAME=$1

if [ $APPNAME = "speed-dreams-2" ] || [ $APPNAME = "fgfs" ] || [ $APPNAME = "vegastrike" ] || [ $APPNAME = "eclipse" ] || [ $APPNAME = "PillarsOfEternity" ]
then
	if [ $APPNAME = "speed-dreams-2" ]
	then
		../bin/gatherer /usr/games/speed-dreams-2
	elif [ $APPNAME = "fgfs" ]
	then
		../bin/gatherer /usr/games/fgfs
	elif [ $APPNAME = "vegastrike" ]
	then
		../bin/gatherer /usr/games/vegastrike
	elif [ $APPNAME = "eclipse" ]
	then
		../bin/gatherer /home/melody/eclipse/java-neon/eclipse/eclipse
	elif [ $APPNAME = "PillarsOfEternity" ]
	then
		../bin/gatherer /usr/games/PillarsOfEternity
	fi
else
	echo "usage: ./gatherer.sh <appname>"
fi

