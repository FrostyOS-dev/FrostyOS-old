#!/bin/sh

make toolchain

$HOME/opt/x86_64-worldos-cross/bin/x86_64-worldos-addr2line -e $1 $2