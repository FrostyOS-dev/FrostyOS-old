#!/bin/bash

make toolchain

toolchain/local/bin/x86_64-worldos-nm -C --format=bsd $1