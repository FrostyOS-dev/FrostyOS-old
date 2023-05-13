#!/bin/bash

# exit on error
set -e

# setup definitions
export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

# make dirs and enter toolchain dir
mkdir -p $PREFIX
rm -fr $HOME/toolchain-build # attempt to remove toolchain directory. If it doesn't exist, it does nothing.
mkdir -p $HOME/toolchain-build
cd $HOME/toolchain-build

if (!($PREFIX/bin/$TARGET-ld -v | grep 2.40 > /dev/null)) then
    echo -----------------
    echo Building binutils
    echo -----------------

    # fetch binutils code
    curl -OL https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz
    tar -xf binutils-2.40.tar.gz

    # build binutils
    cd binutils-2.40
    mkdir build
    cd build
    ../configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
    make -j4
    make install
    cd ../..
else
    echo Binutils is up to date
fi

currentver="$($PREFIX/bin/$TARGET-gcc -dumpversion)"
requiredver="13.1.0"
if [ "$(printf '%s\n' "$requiredver" "$currentver" | sort -V | head -n1)" = "$requiredver" ]; then 
    echo GCC is up to date.
else
    echo ------------
    echo Building gcc
    echo ------------

    # fetch gcc code
    curl -OL https://ftp.gnu.org/gnu/gcc/gcc-13.1.0/gcc-13.1.0.tar.gz
    tar -xf gcc-13.1.0.tar.gz

    # build gcc
    cd gcc-13.1.0
    mkdir build
    cd build
    ../configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
    make all-gcc -j4
    make all-target-libgcc -j4
    make install-gcc
    make install-target-libgcc
    cd ../..
fi

# clean-up
cd ..
rm -fr $HOME/toolchain-build