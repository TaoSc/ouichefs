#! /bin/bash

# Fixer les variables avec les chemins de vos fichiers
HDA="-drive file=/home/louis/Documents/pnl-vm/pnl-tp-2021.img,format=raw"
HDB="-drive file=/home/louis/Documents/pnl-vm/myHome.img,format=raw"
HDC="-drive file=/home/louis/Documents/pnl-vm/test.img,format=raw"
SHARED=/home/louis/Documents/pnl/ouichefs
KERNEL=/home/louis/Documents/pnl-vm/linux-5.10.17/arch/x86/boot/bzImage

# Linux kernel options
CMDLINE="root=/dev/sda1 rw console=ttyS0 kgdboc=ttyS1"

FLAGS="--enable-kvm "
VIRTFS+=" --virtfs local,path=${SHARED},mount_tag=share,security_model=passthrough,id=share "

exec qemu-system-x86_64 ${FLAGS} \
     ${HDA} ${HDB} ${HDC} \
     ${VIRTFS} \
     -net user -net nic \
     -serial mon:stdio \
     -serial tcp::1234,server,nowait \
     -boot c -m 2G \
     -kernel "${KERNEL}" -append "${CMDLINE}"