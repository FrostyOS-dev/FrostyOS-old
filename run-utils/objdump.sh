#!/bin/sh

make toolchain

$HOME/opt/x86_64-worldos-cross/bin/x86_64-worldos-objdump -D -Mx86-64,intel $1 > dump