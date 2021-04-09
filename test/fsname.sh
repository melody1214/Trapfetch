#!/bin/bash

FSNAME=$(eval $(lsblk -oMOUNTPOINT,PKNAME -P -M | grep 'MOUNTPOINT="/"'); echo $PKNAME)

echo $FSNAME
