#!/usr/bin/env bash

insmod /share/src/ouichefs.ko
mkdir /wish 2>/dev/null
mount -t ouichefs /dev/sdc /wish
mknod /dev/ouichefs_ioctl c 248 0
cd /wish
