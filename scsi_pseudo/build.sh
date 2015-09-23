$NAME=scsi_pseudo
gcc -D_KERNEL -m64 -mcmodel=kernel -mno-red-zone -ffreestanding -nodefaultlibs -c $NAME.c
ld -r -o $NAME $NAME.o

cp $NAME.conf /usr/kernel/drv
cp $NAME /tmp
ln -sf /tmp/$NAME /usr/kernel/drv/amd64/$NAME
add_drv $NAME
