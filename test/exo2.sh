#!/usr/bin/env bash

echo "START TEST EXO 2"
insmod /share/src/ouichefs.ko
mkdir /wish 2>/dev/null
mount -t ouichefs /dev/sdc /wish
cd /wish

echo
ls
echo

echo "ver1" > test_file1
echo "ver1" > test_file2
echo "ver2" > test_file1
echo "ver3" > test_file1

echo "ver1" > test_file3
echo "ver2" > test_file3
echo "ver3" > test_file3

echo
ls
echo

cat /sys/kernel/debug/ouichefs
echo

rm test_file1 test_file2 test_file3
echo "[            ] ouichefs : removing test files"

#cat /sys/kernel/debug/ouichefs
#echo

cd /
umount /wish
rmmod ouichefs

echo "END TEST EXO 2"
