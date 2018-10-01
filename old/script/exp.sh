#!/bin/sh

APPNAME=$1
EXPPATH='/home/melody/study/project/bp/experiments/blktrace'
CWD=$PWD

if [ $APPNAME = "speed-dreams-2" ] || [ $APPNAME = "fgfs" ] || [ $APPNAME = "vegastrike" ] || [ $APPNAME = "eclipse" ]
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



if [ ! -d $EXPPATH/$APPNAME ]
then
	mkdir $EXPPATH/$APPNAME
fi

if [ ! -d $EXPPATH/$APPNAME/origin ]
then
	mkdir $EXPPATH/$APPNAME/origin
fi

if [ ! -d $EXPPATH/$APPNAME/subst ]
then
	mkdir $EXPPATH/$APPNAME/subst
fi

if [ ! -d $EXPPATH/$APPNAME/sorted ]
then
	mkdir $EXPPATH/$APPNAME/sorted
fi

if [ ! -d $EXPPATH/$APPNAME/btt ]
then
	mkdir $EXPPATH/$APPNAME/btt
fi

if [ ! -d $EXPPATH/$APPNAME/iowatcher ]
then
	mkdir $EXPPATH/$APPNAME/iowatcher
fi

if [ ! -d $EXPPATH/$APPNAME/trace ]
then
	mkdir $EXPPATH/$APPNAME/trace
fi

if [ $3 -lt 6 ]
then
	if [ $2 = "pf" ]
	then
		blkparse ../blkout | grep I | grep R > $EXPPATH/$APPNAME/origin/pf.$APPNAME.blktrace.$3
		blkparse -i ../blkout.blktrace* -d pf.$APPNAME.$3.bin -O
		blkrawverify ../blkout
		btt -o $EXPPATH/$APPNAME/btt/pf.btt.$3 -i pf.$APPNAME.$3.bin -B bno -A	
		iowatcher -t pf.$APPNAME.$3.bin -o $EXPPATH/$APPNAME/iowatcher/pf.iowatcher.$3.svg
		#bno_plot bno*
		
		rm -rf bno* *iops* sys_* *mbps* ../blkout.*
		mv pf.$APPNAME.$3.bin $EXPPATH/$APPNAME/trace/pf.$APPNAME.$3.bin

	elif [ $2 = "normal" ]
	then
		blkparse ../blkout | grep I | grep R > $EXPPATH/$APPNAME/origin/normal.$APPNAME.blktrace.$3
		blkparse -i ../blkout.blktrace* -d normal.$APPNAME.$3.bin -O
		blkrawverify ../blkout
		btt -o $EXPPATH/$APPNAME/btt/normal.btt.$3 -i normal.$APPNAME.$3.bin -B bno -A	
		iowatcher -t normal.$APPNAME.$3.bin -o $EXPPATH/$APPNAME/iowatcher/normal.iowatcher.$3.svg
		#bno_plot bno*
		
		rm -rf bno* *iops* sys_* *mbps* ../blkout.*
		mv normal.$APPNAME.$3.bin $EXPPATH/$APPNAME/trace/normal.$APPNAME.$3.bin
	fi
fi

if [ $3 -eq 5 ] && [ $2 = "pf" ]
then
	for i in 1 2 3 4 5
	do
		sed -e 's/+/\t/' -e "/$APPNAME\|prefetch/!d" $EXPPATH/$APPNAME/origin/normal.$APPNAME.blktrace.$i > $EXPPATH/$APPNAME/subst/normal.$APPNAME.blktrace.$i
		sed -e 's/+/\t/' -e "/$APPNAME\|prefetch/!d" $EXPPATH/$APPNAME/origin/pf.$APPNAME.blktrace.$i > $EXPPATH/$APPNAME/subst/pf.$APPNAME.blktrace.$i
	done

	for i in 1 2 3 4 5
	do
		sort -n -k 8 $EXPPATH/$APPNAME/subst/normal.$APPNAME.blktrace.$i -o $EXPPATH/$APPNAME/sorted/normal.$APPNAME.blktrace.$i
		sort -n -k 8 $EXPPATH/$APPNAME/subst/pf.$APPNAME.blktrace.$i -o $EXPPATH/$APPNAME/sorted/pf.$APPNAME.blktrace.$i
	done

	cd $EXPPATH/$APPNAME/trace
	for i in 1 2 3 4 5
	do
		iowatcher -t normal.$APPNAME.$i.bin -t pf.$APPNAME.$i.bin -l Normal -l Prefetch -o $EXPPATH/$APPNAME/iowatcher/comp.iowatcher.$i.svg
	done

	chown -R $(whoami):$(whoami) $EXPPATH/$APPNAME/*	
	cd $CWD
	
	python trimer.py $APPNAME
fi
