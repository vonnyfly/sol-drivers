gcc -D_KERNEL -m64 -mcmodel=kernel -mno-red-zone -ffreestanding -nodefaultlibs -c dummy.c
ld -r -o dummy dummy.o

cp dummy.conf /usr/kernel/drv
cp dummy /tmp
ln -sf /tmp/dummy /usr/kernel/drv/amd64/dummy
add_drv dummy
