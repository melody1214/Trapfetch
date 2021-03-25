#!/bin/sh

APPNAME=$1
EXPPATH='/home/melody/work/trapfetch/experiments/blktrace'
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

if [ $3 -lt 3 ]
then
	if [ $2 = "pf" ]
	then
		blkparse ../blkout > $EXPPATH/$APPNAME/origin/pf.$APPNAME.blktrace.$3
		blkparse -i ../blkout.blktrace* -d pf.$APPNAME.$3.bin -O
		blkrawverify ../blkout
		btt -o $EXPPATH/$APPNAME/btt/pf.btt.$3 -i pf.$APPNAME.$3.bin -B bno -A	
		iowatcher -t pf.$APPNAME.$3.bin -o $EXPPATH/$APPNAME/iowatcher/pf.iowatcher.$3.svg
#		if [ $3 -eq 2 ]
#		then
#			iowatcher -t pf.$APPNAME.$3.bin --movie -o pf.iowatcher.$3.mp4
#			mv pf.iowatcher.$3.mp4 $EXPPATH/$APPNAME/iowatcher/pf.iowatcher.$3.mp4
#			rm -rf ./io-movie*
#		fi
		#bno_plot bno*
		
		rm -rf bno* *iops* sys_* *mbps*
		mv pf.$APPNAME.$3.bin $EXPPATH/$APPNAME/trace/pf.$APPNAME.$3.bin

		sed -e 's/+/\t/' -e "/$APPNAME\|caller/!d" $EXPPATH/$APPNAME/origin/pf.$APPNAME.blktrace.$3 > $EXPPATH/$APPNAME/subst/pf.$APPNAME.blktrace.$3
		sort -n -k 8 $EXPPATH/$APPNAME/subst/pf.$APPNAME.blktrace.$3 -o $EXPPATH/$APPNAME/sorted/pf.$APPNAME.blktrace.$3

	elif [ $2 = "normal" ]
	then
		blkparse ../blkout > $EXPPATH/$APPNAME/origin/normal.$APPNAME.blktrace.$3
		blkparse -i ../blkout.blktrace* -d normal.$APPNAME.$3.bin -O
		blkrawverify ../blkout
		btt -o $EXPPATH/$APPNAME/btt/normal.btt.$3 -i normal.$APPNAME.$3.bin -B bno -A	
		iowatcher -t normal.$APPNAME.$3.bin -o $EXPPATH/$APPNAME/iowatcher/normal.iowatcher.$3.svg
#		if [ $3 -eq 2 ]
#		then
#			iowatcher -t normal.$APPNAME.$3.bin --movie -o normal.iowatcher.$3.mp4
#			mv normal.iowatcher.$3.mp4 $EXPPATH/$APPNAME/iowatcher/normal.iowatcher.$3.mp4
#			rm -rf ./io-movie*
#		fi
#		#bno_plot bno*
		
		rm -rf bno* *iops* sys_* *mbps*
		mv normal.$APPNAME.$3.bin $EXPPATH/$APPNAME/trace/normal.$APPNAME.$3.bin

		sed -e 's/+/\t/' -e "/$APPNAME\|caller/!d" $EXPPATH/$APPNAME/origin/normal.$APPNAME.blktrace.$3 > $EXPPATH/$APPNAME/subst/normal.$APPNAME.blktrace.$3
		sort -n -k 8 $EXPPATH/$APPNAME/subst/normal.$APPNAME.blktrace.$3 -o $EXPPATH/$APPNAME/sorted/normal.$APPNAME.blktrace.$3

	fi
fi

if [ $3 -eq 2 ] && [ $2 = "pf" ]
then
	cd $EXPPATH/$APPNAME/trace
	for i in 1 2
	do
		iowatcher -t normal.$APPNAME.$i.bin -t pf.$APPNAME.$i.bin -l Normal -l Prefetch -T $APPNAME -o $EXPPATH/$APPNAME/iowatcher/comp.iowatcher.$i.svg
	done

#iowatcher -t normal.$APPNAME.1.bin -t pf.$APPNAME.1.bin -l Normal -l Prefetch -T $APPNAME --movie -o comp.iowatcher.1.mp4
#	iowatcher -t normal.$APPNAME.2.bin -t pf.$APPNAME.2.bin -l Normal -l Prefetch -T $APPNAME --movie=rect -o comp.iowatcher.2.mp4
	
#	mv comp.iowatcher.1.mp4 $EXPPATH/$APPNAME/iowatcher/comp.iowatcher.1.mp4
#	mv comp.iowatcher.2.mp4 $EXPPATH/$APPNAME/iowatcher/comp.iowatcher.2.mp4

#	rm -rf ./io-movie*	

	chown -R melody:melody $EXPPATH/$APPNAME/*	
	cd $CWD
	
	python trimer_bp.py $APPNAME
fi
