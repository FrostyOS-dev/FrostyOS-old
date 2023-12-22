#!/bin/sh

make toolchain

toolchain/local/bin/x86_64-worldos-objdump -D -Mx86-64,intel $1 > dump