#!/usr/bin/env bash
insmod /share/ouichefs.ko
mkdir /wish
mount -t ouichefs /dev/sdc /wish
cd /wish