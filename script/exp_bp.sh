#!/bin/sh

APPNAME=$1
EXPPATH='/home/melody/work/trapfetch/experiments/blktrace'
PATH_PREFETCHER=/home/melody/work/trapfetch/prefetcher/prefetcher.x86_64
PATH_EXP=/home/melody/work/trapfetch/exp
FSNAME=$(eval $(lsblk -oMOUNTPOINT,PKNAME -P -M | grep 'MOUNTPOINT="/"'); echo $PKNAME)
CWD=$PWD

if [ $APPNAME = "speed-dreams-2" ] || [ $APPNAME = "fgfs" ] || [ $APPNAME = "vegastrike" ] || [ $APPNAME = "eclipse" ] || [ $APPNAME = "PillarsOfEternity" ] || [ $APPNAME = "firefox" ] || [ $APPNAME = "SOMA" ] || [ $APPNAME = "WL2" ] || [ $APPNAME = "soffice" ]
then
	if [ $2 = "pf" ] || [ $2 = "normal" ]
	then
		if [ ! $3 -lt 11 ]
		then
			echo "usage: ./exp.sh <appname> <pf or normal> <0 to 10>"
			exit
		fi
	else
		echo "usage: ./exp.sh <appname> <pf or normal> <0 to 10>"
		exit
	fi
else
	echo "usage: ./exp.sh <appname> <pf or normal> <0 to 10>"
	exit
fi



if [ ! -d $PATH_EXP/$APPNAME ]
then
	mkdir -p $PATH_EXP/$APPNAME
fi

if [ ! -d $PATH_EXP/$APPNAME/origin ]
then
	mkdir -p $PATH_EXP/$APPNAME/origin
fi

if [ ! -d $PATH_EXP/$APPNAME/subst ]
then
	mkdir -p $PATH_EXP/$APPNAME/subst
fi

if [ ! -d $PATH_EXP/$APPNAME/sorted ]
then
	mkdir -p $PATH_EXP/$APPNAME/sorted
fi

if [ ! -d $PATH_EXP/$APPNAME/btt ]
then
	mkdir -p $PATH_EXP/$APPNAME/btt
fi

if [ ! -d $PATH_EXP/$APPNAME/iowatcher ]
then
	mkdir -p $PATH_EXP/$APPNAME/iowatcher
fi

if [ ! -d $PATH_EXP/$APPNAME/trace ]
then
	mkdir -p $PATH_EXP/$APPNAME/trace
fi

if [ $3 -lt 3 ]
then
	if [ $2 = "pf" ]
	then
		blkparse $PATH_EXP/$APPNAME/blkout > $PATH_EXP/$APPNAME/origin/pf.$APPNAME.blktrace.$3
		blkparse -i $PATH_EXP/$APPNAME/blkout.blktrace* -d pf.$APPNAME.$3.bin -O
		blkrawverify $PATH_EXP/$APPNAME/blkout
		btt -o $PATH_EXP/$APPNAME/btt/pf.btt.$3 -i pf.$APPNAME.$3.bin -B bno -A	
		iowatcher -t pf.$APPNAME.$3.bin -o $PATH_EXP/$APPNAME/iowatcher/pf.iowatcher.$3.svg
#		if [ $3 -eq 2 ]
#		then
#			iowatcher -t pf.$APPNAME.$3.bin --movie -o pf.iowatcher.$3.mp4
#			mv pf.iowatcher.$3.mp4 $PATH_EXP/$APPNAME/iowatcher/pf.iowatcher.$3.mp4
#			rm -rf ./io-movie*
#		fi
		#bno_plot bno*
		
		rm -rf bno* *iops* sys_* *mbps*
		mv pf.$APPNAME.$3.bin $PATH_EXP/$APPNAME/trace/pf.$APPNAME.$3.bin

		sed -e 's/+/\t/' -e '/PillarsOfEterni\|prefetcher.x86_/!d' $PATH_EXP/$APPNAME/origin/pf.$APPNAME.blktrace.$3 > $PATH_EXP/$APPNAME/subst/pf.$APPNAME.blktrace.$3
		sort -n -k 8 $PATH_EXP/$APPNAME/subst/pf.$APPNAME.blktrace.$3 -o $PATH_EXP/$APPNAME/sorted/pf.$APPNAME.blktrace.$3

	elif [ $2 = "normal" ]
	then
		blkparse $PATH_EXP/$APPNAME/blkout > $PATH_EXP/$APPNAME/origin/normal.$APPNAME.blktrace.$3
		blkparse -i $PATH_EXP/$APPNAME/blkout.blktrace* -d normal.$APPNAME.$3.bin -O
		blkrawverify $PATH_EXP/$APPNAME/blkout
		btt -o $PATH_EXP/$APPNAME/btt/normal.btt.$3 -i normal.$APPNAME.$3.bin -B bno -A	
		iowatcher -t normal.$APPNAME.$3.bin -o $PATH_EXP/$APPNAME/iowatcher/normal.iowatcher.$3.svg
#		if [ $3 -eq 2 ]
#		then
#			iowatcher -t normal.$APPNAME.$3.bin --movie -o normal.iowatcher.$3.mp4
#			mv normal.iowatcher.$3.mp4 $PATH_EXP/$APPNAME/iowatcher/normal.iowatcher.$3.mp4
#			rm -rf ./io-movie*
#		fi
#		#bno_plot bno*
		
		rm -rf bno* *iops* sys_* *mbps*
		mv normal.$APPNAME.$3.bin $PATH_EXP/$APPNAME/trace/normal.$APPNAME.$3.bin

		sed -e 's/+/\t/' -e '/PillarsOfEterni\|prefetcher.x86_/!d' $PATH_EXP/$APPNAME/origin/normal.$APPNAME.blktrace.$3 > $PATH_EXP/$APPNAME/subst/normal.$APPNAME.blktrace.$3
		sort -n -k 8 $PATH_EXP/$APPNAME/subst/normal.$APPNAME.blktrace.$3 -o $PATH_EXP/$APPNAME/sorted/normal.$APPNAME.blktrace.$3

	fi
fi

if [ $3 -eq 2 ] && [ $2 = "pf" ]
then
	cd $PATH_EXP/$APPNAME/trace
	for i in 1 2
	do
		iowatcher -t normal.$APPNAME.$i.bin -t pf.$APPNAME.$i.bin -l Normal -l Prefetch -T $APPNAME -o $PATH_EXP/$APPNAME/iowatcher/comp.iowatcher.$i.svg
	done

#iowatcher -t normal.$APPNAME.1.bin -t pf.$APPNAME.1.bin -l Normal -l Prefetch -T $APPNAME --movie -o comp.iowatcher.1.mp4
#	iowatcher -t normal.$APPNAME.2.bin -t pf.$APPNAME.2.bin -l Normal -l Prefetch -T $APPNAME --movie=rect -o comp.iowatcher.2.mp4
	
#	mv comp.iowatcher.1.mp4 $PATH_EXP/$APPNAME/iowatcher/comp.iowatcher.1.mp4
#	mv comp.iowatcher.2.mp4 $PATH_EXP/$APPNAME/iowatcher/comp.iowatcher.2.mp4

#	rm -rf ./io-movie*	

	chown -R melody:melody $PATH_EXP/$APPNAME/*	
	cd $CWD
	
	python trimer_bp.py $APPNAME
fi
