#!/usr/bin/env bash

cmake . \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX="../../../../target/classes/linux64" \
-DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake

make
r1=$?

mkdir -p ../../../../target/classes/linux64/
cp Play50hz-player.bin ../../../../target/classes/linux64/Play50hz-player.bin

exit ${r1}
