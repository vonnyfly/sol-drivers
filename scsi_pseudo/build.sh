#!/bin/sh

arch=""
extra=""
if [[ `uname -i` = "i86pc" ]];then
	arch="amd64"
	extra="-mcmodel=kernel -mno-red-zone"
else
	arch="sparcv9"
	extra="-mcpu=v9 -mcmodel=medlow -fno-pic -mno-fpu"
fi
NAME=scsi_pseudo
gcc -D_KERNEL -m64 -ffreestanding -nodefaultlibs $extra -c $NAME.c
ld -r -o $NAME $NAME.o
