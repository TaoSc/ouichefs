#!/usr/bin/env bash

insmod /share/ouichefs.ko
mkdir /wish 2>/dev/null
mount -t ouichefs /dev/sdc /wish
cd /wish