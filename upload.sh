#!/bin/bash
set -o nounset          # Treat unset variables as an error

rsync -avuP ./ llfeng@x12-0:~/sol-drivers/
ssh llfeng@12-0 "cd ~/sol-drivers/scsi_pseudo;./build.sh; scp -r ~/sol-drivers/* root@hotcat:~/sol-drivers"
ssh root@hotcat "cd ~/sol-drivers/scsi_pseudo;./install.sh"


