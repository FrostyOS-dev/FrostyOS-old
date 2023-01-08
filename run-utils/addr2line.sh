#!/bin/sh

make toolchain

$HOME/opt/cross/bin/x86_64-elf-addr2line -e $1 $2