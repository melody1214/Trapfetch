#!/bin/sh

APPNAME=$1

if [ $APPNAME = "speed-dreams-2" ] || [ $APPNAME = "fgfs" ] || [ $APPNAME = "vegastrike" ] || [ $APPNAME = "eclipse" ] || [ $APPNAME = "PillarsOfEternity" ] || [ $APPNAME = "firefox" ] || [ $APPNAME = "SOMA" ] || [ $APPNAME = "swriter" ] || [ $APPNAME = "impress" ] || [ $APPNAME = "witcher2" ] || [ $APPNAME = "longdark" ] || [ $APPNAME = "WL2" ] || [ $APPNAME = "EoCApp" ]
then
	if [ $APPNAME = "longdark" ]
	then
		echo 1794125064 > /proc/lba_check #launch
# echo 1861728808 > /proc/lba_check #loading_1
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /home/melody/The_Long_Dark/TheLongDark/tld.x86_64
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/The_Long_Dark/TheLongDark/tld.x86_64
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /home/melody/The_Long_Dark/TheLongDark/tld.x86_64
		fi
	elif [ $APPNAME = "EoCApp" ]
	then
		echo 1771265912 > /proc/lba_check #launch
#echo 1770917672 > /proc/lba_check #loading_1
#echo 1774163240 > /proc/lba_check #loading_2
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /home/melody/GOG\ Games/Divinity\ Original\ Sin\ Enhanced\ Edition/game/EoCApp
		elif [ $2 = "cold" ]
		then
			export "LD_LIBRARY_PATH=/home/melody/GOG Games/Divinity Original Sin Enhanced Edition/game/"
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/GOG\ Games/Divinity\ Original\ Sin\ Enhanced\ Edition/game/EoCApp
		elif [ $2 = "warm" ]
		then
			export "LD_LIBRARY_PATH=/home/melody/GOG Games/Divinity Original Sin Enhanced Edition/game/"
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /home/melody/GOG\ Games/Divinity\ Original\ Sin\ Enhanced\ Edition/game/EoCApp
		fi
	elif [ $APPNAME = "WL2" ]
	then
echo 1024367544 > /proc/lba_check #launch
#echo 1074907728 > /proc/lba_check #loading_1
#echo 1027991040 > /proc/lba_check #loading_2
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_32 /home/melody/Games/Wasteland2/game/WL2
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/Games/Wasteland2/game/WL2
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /home/melody/Games/Wasteland2/game/WL2
		fi
	elif [ $APPNAME = "witcher2" ]
	then
#echo 1155192544 > /proc/lba_check #launch
#echo 1186953480 > /proc/lba_check #loading_1
		echo 1185814992 > /proc/lba_check #loading_2_start
		echo 1183045888 > /proc/lbn_check_2 #loading_2_end
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_32 /home/melody/GOG\ Games/The\ Witcher\ 2\ Assassins\ Of\ Kings\ Enhanced\ Edition/game/witcher2
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/GOG\ Games/The\ Witcher\ 2\ Assassins\ Of\ Kings\ Enhanced\ Edition/game/witcher2
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /home/melody/GOG\ Games/The\ Witcher\ 2\ Assassins\ Of\ Kings\ Enhanced\ Edition/game/witcher2
		fi
	elif [ $APPNAME = "impress" ]
	then
		echo 1085671408 > /proc/lba_check #launch
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /usr/lib/libreoffice/program/soffice.bin --impress
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/usr/lib/libreoffice/program/soffice.bin --impress
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /usr/lib/libreoffice/program/soffice.bin --impress
		fi
	elif [ $APPNAME = "swriter" ]
	then
		echo 1085385264 > /proc/lba_check #launch
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /usr/lib/libreoffice/program/soffice.bin --writer
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/usr/lib/libreoffice/program/soffice.bin --writer
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /usr/lib/libreoffice/program/soffice.bin --writer
		fi
	elif [ $APPNAME = "SOMA" ]
	then
	echo 1118438128 > /proc/lba_check	#launch
#echo 1198180224 > /proc/lbn_check_2 #loading_1
#echo 1231488352 > /proc/lba_check	#loading_2
		cd /home/melody/GOG\ Games/SOMA/game
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /home/melody/GOG\ Games/SOMA/game/Soma.bin.x86_64
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/GOG\ Games/SOMA/game/Soma.bin.x86_64
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /home/melody/GOG\ Games/SOMA/game/Soma.bin.x86_64
		fi
		cd /home/melody/work/trapfetch/old
	elif [ $APPNAME = "speed-dreams-2" ]
	then
		echo 937575032 > /proc/lba_check	#launch
#echo 1436236008 > /proc/lba_check	#loading_1
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /usr/games/speed-dreams-2
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/coldstart /usr/games/speed-dreams-2
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /usr/games/speed-dreams-2
		fi
	elif [ $APPNAME = "fgfs" ]
	then
		if [ $2 = "pf" ]
		then
			sudo -u melody echo 1926056856 | sudo tee /proc/lba_check > /dev/null
			sudo -u melody echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /usr/games/fgfs
		elif [ $2 = "cold" ]
		then
			sudo -u melody echo 1926056856 | sudo tee /proc/lba_check > /dev/null
			sudo -u melody echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null
			sync
			sleep 2
			cat /proc/uptime
			/usr/games/fgfs
		elif [ $2 = "warm" ]
		then
			sudo -u melody echo 1926056856 | sudo tee /proc/lba_check > /dev/null
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /usr/games/fgfs
		fi
	elif [ $APPNAME = "vegastrike" ]
	then
		#echo 1533787600 > /proc/lba_check	#launch
		echo 976374648 > /proc/lba_check	#loading_1
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /usr/games/vegastrike
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/usr/games/vegastrike
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /usr/games/vegastrike
		fi
	elif [ $APPNAME = "eclipse" ]
	then
		echo 1537885968 > /proc/lba_check
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /home/melody/eclipse/java-neon/eclipse/eclipse
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/eclipse/java-neon/eclipse/eclipse
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /home/melody/eclipse/java-neon/eclipse/eclipse
		fi
	elif [ $APPNAME = "PillarsOfEternity" ]
	then
		# echo 1799594280 > /proc/lba_check	#launch
		 echo 1923679968 > /proc/lba_check	#loading_1
		# echo 1794714528 > /proc/lba_check	#loading_2
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /usr/games/PillarsOfEternity
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/coldstart /usr/games/PillarsOfEternity
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /usr/games/PillarsOfEternity
		fi
	elif [ $APPNAME = "firefox" ]
	then
		echo 1781246456 > /proc/lba_check	#launch
		
		if [ $2 = "pf" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/caller_64 /usr/lib/firefox/firefox
		elif [ $2 = "cold" ]
		then
			echo 3 > /proc/sys/vm/drop_caches
			sync
			sleep 2
			cat /proc/uptime
			/usr/lib/firefox/firefox
		elif [ $2 = "warm" ]
		then
			cat /proc/uptime
			/home/melody/work/trapfetch/old/bin/warmstart /usr/lib/firefox/firefox
		fi
	fi
else
	echo "usage : ./blktrace.sh <appname> <pf or normal> <sda or sdb>"
fi

