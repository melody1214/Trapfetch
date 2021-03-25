#!/bin/sh

APPNAME=$1
LOGPATH='/home/melody/work/trapfetch/logs'

if [ $APPNAME = "speed-dreams-2" ] || [ $APPNAME = "fgfs" ] || [ $APPNAME = "vegastrike" ] || [ $APPNAME = "eclipse" ] || [ $APPNAME = "PillarsOfEternity" ] || [ $APPNAME = "Soma.bin.x86_64" ] || [ $APPNAME = "witcher2" ] || [ $APPNAME = "soffice.bin" ] || [ $APPNAME = "wesnoth" ] || [ $APPNAME = "pyrogenesis" ] || [ $APPNAME = "firefox" ] || [ $APPNAME = "tld.x86_64" ] || [ $APPNAME = "Silence.x86_64" ] || [ $APPNAME = "WL2" ] || [ $APPNAME = "EoCApp" ]
then
	sort -t , -k 1,1 -u $LOGPATH/log_candidates | sort -t , -n -k 2,2 > $LOGPATH/etc_$APPNAME
	sort -t , -n -k 2,2 $LOGPATH/log > $LOGPATH/read_$APPNAME
else
	echo "usage: ./sort.sh <appname>"
fi
