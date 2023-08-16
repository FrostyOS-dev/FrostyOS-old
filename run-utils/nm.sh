#!/bin/bash

make toolchain

$HOME/opt/x86_64-worldos-cross/bin/x86_64-worldos-nm -C --format=bsd $1