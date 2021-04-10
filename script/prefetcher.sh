#!/bin/sh

APPNAME=$1
PATH_ECLIPSE=/home/melody/eclipse/java-2021-03/eclipse/eclipse
PATH_ANDROIDSTUDIO=/home/melody/android-studio/bin/studio.sh
PATH_ONLYOFFICE_DOC=/usr/bin/onlyoffice-desktopeditors

if [ $APPNAME = "speed-dreams-2" ] || [ $APPNAME = "ARK" ] || [ $APPNAME = "onlyoffice-doc" ] || [ $APPNAME = "studio" ] || [ $APPNAME = "fgfs" ] || [ $APPNAME = "vegastrike" ] || [ $APPNAME = "eclipse" ] || [ $APPNAME = "PillarsOfEternity" ] || [ $APPNAME = "firefox" ] || [ $APPNAME = "SOMA" ] || [ $APPNAME = "swriter" ] || [ $APPNAME = "impress" ] || [ $APPNAME = "witcher2" ] || [ $APPNAME = "longdark" ] || [ $APPNAME = "WL2" ] || [ $APPNAME = "EoCApp" ]
then
	if [ $APPNAME = "studio" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 $PATH_ANDROIDSTUDIO
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/bin/sh $PATH_ANDROIDSTUDIO
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/bin/sh $PATH_ANDROIDSTUDIO
		fi
	elif [ $APPNAME = "onlyoffice-doc" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 $PATH_ONLYOFFICE_DOC --new=doc
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			$PATH_ONLYOFFICE_DOC --new=doc
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			$PATH_ONLYOFFICE_DOC --new=doc
		fi
	elif [ $APPNAME = "ARK" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /home/melody/GOG\ Games/ARK/start.sh
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/home/melody/GOG\ Games/ARK/start.sh
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/GOG\ Games/ARK/start.sh
		fi
	elif [ $APPNAME = "longdark" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /home/melody/GOG\ Games/The\ Long\ Dark/tld
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/home/melody/GOG\ Games/The\ Long\ Dark/tld
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/GOG\ Games/The\ Long\ Dark/tld
		fi
	elif [ $APPNAME = "EoCApp" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /home/melody/GOG\ Games/Divinity\ Original\ Sin\ Enhanced\ Edition/game/EoCApp
		elif [ $2 = "cold" ]
		then
			export "LD_LIBRARY_PATH=/home/melody/GOG Games/Divinity Original Sin Enhanced Edition/game/"
			cat /proc/uptime
			/home/melody/GOG\ Games/Divinity\ Original\ Sin\ Enhanced\ Edition/game/EoCApp
		elif [ $2 = "warm" ]
		then
			export "LD_LIBRARY_PATH=/home/melody/GOG Games/Divinity Original Sin Enhanced Edition/game/"
			cat /proc/uptime
			/home/melody/GOG\ Games/Divinity\ Original\ Sin\ Enhanced\ Edition/game/EoCApp
		fi
	elif [ $APPNAME = "WL2" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /home/melody/Games/Wasteland2/game/WL2
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/home/melody/Games/Wasteland2/game/WL2
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/Games/Wasteland2/game/WL2
		fi
	elif [ $APPNAME = "witcher2" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /home/melody/GOG\ Games/The\ Witcher\ 2\ Assassins\ Of\ Kings\ Enhanced\ Edition/game/witcher2
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/home/melody/GOG\ Games/The\ Witcher\ 2\ Assassins\ Of\ Kings\ Enhanced\ Edition/game/witcher2
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/GOG\ Games/The\ Witcher\ 2\ Assassins\ Of\ Kings\ Enhanced\ Edition/game/witcher2
		fi
	elif [ $APPNAME = "impress" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /usr/lib/libreoffice/program/simpress
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/usr/lib/libreoffice/program/simpress
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/usr/lib/libreoffice/program/simpress
		fi
	elif [ $APPNAME = "swriter" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /usr/lib/libreoffice/program/swriter
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/usr/lib/libreoffice/program/swriter
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/usr/lib/libreoffice/program/swriter
		fi
	elif [ $APPNAME = "SOMA" ]
	then
		cd /home/melody/GOG\ Games/SOMA/game
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /home/melody/GOG\ Games/SOMA/game/Soma.bin.x86_64
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/home/melody/GOG\ Games/SOMA/game/Soma.bin.x86_64
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/GOG\ Games/SOMA/game/Soma.bin.x86_64
		fi
		cd /home/melody/work/trapfetch/old
	elif [ $APPNAME = "speed-dreams-2" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /usr/games/speed-dreams-2
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/coldstart /usr/games/speed-dreams-2
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/usr/games/speed-dreams-2
		fi
	elif [ $APPNAME = "fgfs" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /usr/games/fgfs
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/usr/games/fgfs
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/usr/games/fgfs
		fi
	elif [ $APPNAME = "vegastrike" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /usr/bin/vegastrike
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/usr/bin/vegastrike
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/usr/bin/vegastrike
		fi
	elif [ $APPNAME = "eclipse" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 $PATH_ECLIPSE
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			$PATH_ECLIPSE
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			$PATH_ECLIPSE
		fi
	elif [ $APPNAME = "PillarsOfEternity" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /home/melody/GOG\ Games/Pillars\ of\ Eternity\ II\ Deadfire/game/PillarsOfEternityII
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/home/melody/GOG\ Games/Pillars\ of\ Eternity\ II\ Deadfire/game/PillarsOfEternityII
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/GOG\ Games/Pillars\ of\ Eternity\ II\ Deadfire/game/PillarsOfEternityII
		fi
	elif [ $APPNAME = "firefox" ]
	then
		if [ $2 = "pf" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64 /usr/lib/firefox/firefox
		elif [ $2 = "cold" ]
		then
			cat /proc/uptime
			/usr/lib/firefox/firefox
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/usr/lib/firefox/firefox
		fi
	fi
else
	echo "usage : ./blktrace.sh <appname> <pf or normal> <sda or sdb>"
fi

