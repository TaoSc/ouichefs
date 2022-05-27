#!/usr/bin/env bash

insmod /share/ouichefs.ko
mkdir /wish
mount -t ouichefs /dev/sdc /wish
cd /wish

echo "ver1" > test_file1
cat test_file1
echo "ver1" > test_file2
cat test_file2
echo "ver2" > test_file1
echo "ver3" > test_file1

cd /
umount ouichefs

mount -t ouichefs /dev/sdc /wish
cd /wish

cat test_file1
cat test_file2
echo "ver2" > test_file2
cat test_file2

cd /
umount /wish

echo
echo "Doit afficher, dans l'ordre :"
echo ver1
echo ver1
echo ver3
echo ver1
echo ver2