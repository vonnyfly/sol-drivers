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
cp $NAME.conf /usr/kernel/drv
cp $NAME /tmp
ln -sf /tmp/$NAME /usr/kernel/drv/$arch/$NAME
add_drv $NAME
