#!/bin/sh

APPNAME=$1
PATH_PREFETCHER=/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64
PATH_EXP=/home/melody/work/trapfetch/exp
FSNAME=$(eval $(lsblk -oMOUNTPOINT,PKNAME -P -M | grep 'MOUNTPOINT="/"'); echo $PKNAME)


if [ $APPNAME = "speed-dreams-2" ] || [ $APPNAME = "fgfs" ] || [ $APPNAME = "vegastrike" ] || [ $APPNAME = "eclipse" ] || [ $APPNAME = "PillarsOfEternity" ] || [ $APPNAME = "firefox" ] || [ $APPNAME = "SOMA" ] || [ $APPNAME = "WL2" ] || [ $APPNAME = "soffice" ]
then
	if [ ! -d $PATH_EXP/$APPNAME ]
	then
		mkdir -p $PATH_EXP/$APPNAME
	fi
	
	sudo -u melody echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null
	sync
	sleep 2
	if [ $APPNAME = "PillarsOfEternity" ]
	then
		if [ $2 = "pf" ]
		then
			sudo blktrace -a read -d /dev/$FSNAME -w 300 -D $PATH_EXP/$APPNAME -o blkout & $PATH_PREFETCHER /home/melody/GOG\ Games/Pillars\ of\ Eternity\ II\ Deadfire/game/PillarsOfEternityII
		elif [ $2 = "normal" ]
		then
			sudo blktrace -a read -d /dev/$FSNAME -w 300 -D $PATH_EXP/$APPNAME -o blkout & /home/melody/GOG\ Games/Pillars\ of\ Eternity\ II\ Deadfire/game/PillarsOfEternityII
		fi
	elif [ $APPNAME = "SOMA" ]
	then
		if [ $2 = "pf" ]
		then
			blktrace -a read -d /dev/$FSNAME -w 200 -o $PATH_EXP/$APPNAME/blkout &
			cd /home/melody/GOG\ Games/SOMA/game
			$PATH_PREFETCHER /home/melody/GOG\ Games/SOMA/Soma.bin.x86_64
		elif [ $2 = "normal" ]
		then
			 blktrace -a read -d /dev/$FSNAME -w 200 -o $PATH_EXP/$APPNAME/blkout &
			cd /home/melody/GOG\ Games/SOMA/game
			/home/melody/GOG\ Games/SOMA/Soma.bin.x86_64
		fi
	fi
else
	echo "usage : ./blktrace.sh <appname> <pf or normal> <sda or sdb>"
fi

