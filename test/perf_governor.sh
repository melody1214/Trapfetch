#!/bin/bash

for i in 0 1 2 3 4 5 6 7
do
	echo $1 > /sys/devices/system/cpu/cpu$i/cpufreq/scaling_governor
done

if $1 == "userspace"
then
	for i in 0 1 2 3 4 5 6 7
	do
		echo 3900000 > /sys/devices/system/cpu/cpu$i/cpufreq/scaling_setspeed
		echo 3900000 > /sys/devices/system/cpu/cpu$i/cpufreq/scaling_max_freq
		echo 3900000 > /sys/devices/system/cpu/cpu$i/cpufreq/scaling_min_freq
	done
fi

cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
cat /sys/devices/system/cpu/cpu*/cpufreq/scaling_cur_freq
