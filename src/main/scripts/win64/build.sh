#!/usr/bin/env bash

cmake . \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX="../../../../target/classes/win64" \
-DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake

make
r1=$?

mkdir -p ../../../../target/classes/win64/
cp Play50hz-player.exe ../../../../target/classes/win64/Play50hz-player.exe

exit ${r1}
