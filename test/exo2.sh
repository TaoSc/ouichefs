#!/usr/bin/env bash
echo "START TEST EXO 2"
insmod /share/ouichefs.ko
mkdir /wish
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
echo "ver4" > test_file3
echo "ver5" > test_file3
echo "ver6" > test_file3

echo
ls
echo

cat /sys/kernel/debug/ouichefs
echo

rm test_file1 test_file2 test_file3
echo "[            ] ouichefs : removing test files"

cat /sys/kernel/debug/ouichefs
                   
cd /
umount /wish
rmmod ouichefs

echo "END TEST EXO 2"


#
