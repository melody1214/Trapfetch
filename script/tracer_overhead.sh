#!/bin/sh

APPNAME=$1
SCENARIO=$2

mem_clean() {
	echo 3 > /proc/sys/vm/drop_caches
	sync
	sleep 2
}

if [ $APPNAME = "firefox" ]
then
	echo 1283150984 > /proc/lba_check #launch

	mem_clean
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /usr/lib/firefox/firefox
elif [ $APPNAME = "PillarsOfEternity" ]
then
	if [ $SCENARIO = "launch" ]
	then
		echo 1283150984 > /proc/lba_check #launch
	elif [ $SCENARIO = "loading1" ]
	then
		echo 1275854016 > /proc/lba_check #loading 1
	elif [ $SCENARIO = "loading2" ]
	then
		echo 1277231808 > /proc/lba_check #loading 2
	fi
	mem_clean
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /usr/games/PillarsOfEternity
elif [ $APPNAME = "fgfs" ]
then
	if [ $SCENARIO = "launch" ]
	then
		echo 1874410720 > /proc/lba_check
	fi

	mem_clean
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /usr/games/fgfs
elif [ $APPNAME = "speed-dreams-2" ]
then
	if [ $SCENARIO = "launch" ]
	then
		echo 1251001224 > /proc/lba_check
	elif [ $SECNARIO = "loading1" ]
	then
		echo 1251129576 > /proc/lba_check
	fi

	mem_clean
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /usr/games/speed-dreams-2
elif [ $APPNAME = "longdark" ]
then
	if [ $SCENARIO = "launch" ]
	then
		echo 1794125064 > /proc/lba_check
	elif [ $SCENARIO = "loading1" ]
	then
		echo 1861728808 > /proc/lba_check
	fi

	mem_clean
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /home/melody/The_Long_Dark/TheLongDark/tld.x86_64
elif [ $APPNAME = "EoCApp" ]
then
	if [ $SCENARIO = "launch" ]
	then
		echo 1775277696 > /proc/lba_check #launch
	elif [ $SCENARIO = "loading1" ]
	then
		echo 1773020952 > /proc/lba_check #loading1
	elif [ $SCENARIO = "loading2" ]
	then
		echo 1774031048 > /proc/lba_check #loading2
	fi

	mem_clean
	export "LD_LIBRARY_PATH=/home/melody/GOG Games/Divinity Original Sin Enhanced Edition/game/"
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /home/melody/GOG\ Games/Divinity\ Original\ Sin\ Enhanced\ Edition/game/EoCApp
elif [ $APPNAME = "eclipse" ]
then
	if [ $SCENARIO = "launch" ]
	then
		echo 1537885968 > /proc/lba_check
	fi

	mem_clean
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /home/melody/eclipse/java-neon/eclipse/eclipse
elif [ $APPNAME = "swriter" ]
then
	if [ $SCENARIO = "launch" ]
	then
		echo 1085385264 > /proc/lba_check #launch
	fi

	mem_clean
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /usr/lib/libreoffice/program/soffice.bin --writer
elif [ $APPNAME = "impress" ]
then
	if [ $SCENARIO = "launch" ]
	then
		echo 1085671408 > /proc/lba_check #launch
	fi
	
	mem_clean
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /usr/lib/libreoffice/program/soffice.bin --impress
elif [ $APPNAME = "SOMA" ]
then
	if [ $SCENARIO = "launch" ]
	then
		echo 1118438128 > /proc/lba_check #launch
	elif [ $SCENARIO = "loading1" ]
	then
		echo 1198180224 > /proc/lba_check #loading1
	elif [ $SCENARIO = "loading2" ]
	then
		echo 1231488352 > /proc/lba_check #loading2
	fi

	mem_clean
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /home/melody/GOG\ Games/SOMA/game/Soma.bin.x86_64
elif [ $APPNAME = "vegastrike" ]
then
	if [ $SCENARIO = "launch" ]
	then
		echo 1424748992 > /proc/lba_check #launch
	elif [ $SCENARIO = "loading1" ]
	then
		echo 1529577760 > /proc/lba_check #loading1
	fi
	
	mem_clean
	cat /proc/uptime
	/home/melody/study/projects/trapfetch/tracer.x86_64 /usr/games/vegastrike
fi



