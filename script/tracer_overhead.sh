#!/bin/sh

APPNAME=$1
SCENARIO=$2
PATH_TRACER=/home/melody/work/trapfetch/tracer/tracer.x86_64
PATH_ONLYOFFICE_DOC=/usr/bin/onlyoffice-desktopeditors

mem_clean() {
	sudo -u melody echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null
	sync
	sleep 2
}

set_flag() {
	cd /home/melody/work/trapfetch/tracer
	make exp
}

if [ $SCENARIO = "syscall" ]
then
	set_flag
	cd $OLDPWD
	mem_clean
elif [ $SCENARIO = "mmap" ]
then
	make tracer
	mem_clean
fi
cat /proc/uptime

if [ $APPNAME = "android-studio" ]
then
	$PATH_TRACER /home/melody/android-studio/android-studio/bin/studio.sh
elif [ $APPNAME = "ARK" ]
then
	$PATH_TRACER /home/melody/GOG\ Games/ARK/start.sh
elif [ $APPNAME = "fgfs" ]
then
	$PATH_TRACER /usr/games/fgfs
elif [ $APPNAME = "onlyoffice-doc" ]
then
	$PATH_TRACER $PATH_ONLYOFFICE_DOC --new=doc
elif [ $APPNAME = "firefox" ]
then
	$PATH_TRACER /usr/lib/firefox/firefox
elif [ $APPNAME = "PillarsOfEternity" ]
then
	$PATH_TRACER /home/melody/GOG\ Games/Pillars\ of\ Eternity\ II\ Deadfire/game/PillarsOfEternityII
elif [ $APPNAME = "fgfs" ]
then
	$PATH_TRACER /usr/games/fgfs
elif [ $APPNAME = "longdark" ]
then
	$PATH_TRACER /home/melody/GOG\ Games/The\ Long\ Dark/tld
elif [ $APPNAME = "EoCApp" ]
then
	export "LD_LIBRARY_PATH=/home/melody/GOG Games/Divinity Original Sin Enhanced Edition/game/"
	$PATH_TRACER /home/melody/GOG\ Games/Divinity\ Original\ Sin\ Enhanced\ Edition/game/EoCApp
elif [ $APPNAME = "eclipse" ]
then
	$PATH_TRACER /home/melody/eclipse/java-2021-03/eclipse/eclipse
elif [ $APPNAME = "swriter" ]
then
	$PATH_TRACER /usr/lib/libreoffice/program/swriter
elif [ $APPNAME = "impress" ]
then
	$PATH_TRACER /usr/lib/libreoffice/program/simpress
elif [ $APPNAME = "SOMA" ]
then
	$PATH_TRACER /home/melody/GOG\ Games/SOMA/game/Soma.bin.x86_64
elif [ $APPNAME = "vegastrike" ]
then
	$PATH_TRACER /usr/games/vegastrike
fi



