#!/bin/bash

make toolchain

$HOME/opt/cross/bin/x86_64-elf-nm -C --format=bsd $1