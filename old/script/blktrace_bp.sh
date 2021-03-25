#!/bin/sh

APPNAME=$1

if [ $APPNAME = "speed-dreams-2" ] || [ $APPNAME = "fgfs" ] || [ $APPNAME = "vegastrike" ] || [ $APPNAME = "eclipse" ] || [ $APPNAME = "PillarsOfEternity" ] || [ $APPNAME = "firefox" ] || [ $APPNAME = "SOMA" ] || [ $APPNAME = "WL2" ] || [ $APPNAME = "soffice" ]
then
	sudo -u melody echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null
	sync
	sleep 2
	if [ $APPNAME = "speed-dreams-2" ]
	then
		if [ $2 = "pf" ]
		then
			 blktrace -a read -w 30 -d /dev/$3 -o ../blkout & ../bin/caller_64 /usr/games/speed-dreams-2
		elif [ $2 = "normal" ]
		then
			 blktrace -a read -d /dev/$3 -w 30 -o ../blkout & /usr/games/speed-dreams-2
		fi
	elif [ $APPNAME = "soffice" ]
	then
		if [ $2 = "pf" ]
		then
			 blktrace -a read -w 20 -d /dev/$3 -o ../blkout & ../bin/caller_64 /usr/lib/libreoffice/program/soffice.bin --writer
		elif [ $2 = "normal" ]
		then
			 blktrace -a read -w 20 -d /dev/$3 -o ../blkout & /usr/lib/libreoffice/program/soffice.bin --writer
		fi
	elif [ $APPNAME = "WL2" ]
	then
		if [ $2 = "pf" ]
		then
			 blktrace -a read -w 100 -d /dev/$3 -o ../blkout & ../bin/caller_32 /home/melody/Games/Wasteland2/game/WL2
		elif [ $2 = "normal" ]
		then
			 blktrace -a read -d /dev/$3 -w 100 -o ../blkout & /home/melody/Games/Wasteland2/game/WL2
		fi
	elif [ $APPNAME = "fgfs" ]
	then
		if [ $2 = "pf" ]
		then
			 sudo blktrace -a read -d /dev/$3 -w 50 -o ../blkout & ../bin/caller_64 /usr/games/fgfs
		elif [ $2 = "normal" ]
		then
			 sudo blktrace -a read -d /dev/$3 -w 50 -o ../blkout & /usr/games/fgfs
		fi
	elif [ $APPNAME = "vegastrike" ]
	then
		if [ $2 = "pf" ]
		then
			 blktrace -a read -d /dev/$3 -w 40 -o ../blkout & ../bin/caller_64 /usr/games/vegastrike
		elif [ $2 = "normal" ]
		then
			 blktrace -a read -d /dev/$3 -w 40 -o ../blkout & /usr/games/vegastrike
		fi
	elif [ $APPNAME = "eclipse" ]
	then
		if [ $2 = "pf" ]
		then
			 blktrace -a read -d /dev/$3 -w 40 -o ../blkout & ../bin/caller_64 /home/melody/eclipse/java-neon/eclipse/eclipse
		elif [ $2 = "normal" ]
		then
			 blktrace -a read -d /dev/$3 -w 40 -o ../blkout & /home/melody/eclipse/java-neon/eclipse/eclipse
		fi
	elif [ $APPNAME = "PillarsOfEternity" ]
	then
		if [ $2 = "pf" ]
		then
			sudo  blktrace -a read -d /dev/$3 -w 80 -o ../blkout & ../bin/caller_64 /usr/games/PillarsOfEternity
		elif [ $2 = "normal" ]
		then
			sudo  blktrace -a read -d /dev/$3 -w 80 -o ../blkout & ../bin/coldstart /usr/games/PillarsOfEternity
		fi
	elif [ $APPNAME = "firefox" ]
	then
		if [ $2 = "pf" ]
		then
			 blktrace -a read -d /dev/$3 -w 20 -o ../blkout & ../bin/caller_64 /usr/lib/firefox/firefox
		elif [ $2 = "normal" ]
		then
			 blktrace -a read -d /dev/$3 -w 20 -o ../blkout & /usr/lib/firefox/firefox
		fi
	elif [ $APPNAME = "SOMA" ]
	then
		if [ $2 = "pf" ]
		then
			 blktrace -a read -d /dev/$3 -w 200 -o ../blkout &
			cd /home/melody/GOG\ Games/SOMA/game
			/home/melody/work/trapfetch/old/bin/caller_64 ./Soma.bin.x86_64
			cd $OLDPWD
		elif [ $2 = "normal" ]
		then
			 blktrace -a read -d /dev/$3 -w 200 -o ../blkout &
			cd /home/melody/GOG\ Games/SOMA/game
			./Soma.bin.x86_64
			cd $OLDPWD
		fi
	fi
else
	echo "usage : ./blktrace.sh <appname> <pf or normal> <sda or sdb>"
fi

