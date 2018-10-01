#!/bin/sh

FILENAME=$1

while read -r line
do
	cat $line > /dev/null
done < "$FILENAME"
