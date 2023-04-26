#!/bin/sh

make toolchain

$HOME/opt/cross/bin/x86_64-elf-objdump -D -Mx86-64,intel $1 > dump