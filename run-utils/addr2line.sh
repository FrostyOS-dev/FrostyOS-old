#!/bin/sh

make toolchain

toolchain/local/bin/x86_64-worldos-addr2line -e $1 $2