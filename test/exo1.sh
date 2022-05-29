#!/usr/bin/env bash
echo "START TEST EXO 1"
insmod /share/src/ouichefs.ko
mkdir /wish 2>/dev/null
mount -t ouichefs /dev/sdc /wish
cd /wish

echo
ls
echo

echo "ver1" > test_file1
cat test_file1
echo "ver1" > test_file2
cat test_file2
echo "ver2" > test_file1
echo "ver3" > test_file1

cd /
umount /wish

mount -t ouichefs /dev/sdc /wish
cd /wish

cat test_file1
cat test_file2
echo "ver2" > test_file2
cat test_file2

echo
ls
echo

echo
echo "Doit afficher, dans l'ordre :"
echo "ver1"
echo "ver1"
echo "ver3"
echo "ver1"
echo "ver2"

rm test_file1 test_file2 
echo "[            ] ouichefs : removing test files"

cd /
umount /wish
rmmod ouichefs

echo "END TEST EXO 1"
