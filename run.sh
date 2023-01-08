#!/bin/bash
echo -----------------------
echo Running...
echo -----------------------
if (($# < 1))
then
qemu-system-x86_64 -pflash ovmf/x86-64/OVMF.fd -hda iso/hdimage.bin -m 256M -debugcon stdio
elif (($1 == "debug"))
then
qemu-system-x86_64 -pflash ovmf/x86-64/OVMF.fd -hda iso/hdimage.bin -m 256M -debugcon stdio
elif (($1 == "release"))
then
qemu-system-x86_64 -pflash ovmf/x86-64/OVMF.fd -hda iso/hdimage.bin -m 256M
fi