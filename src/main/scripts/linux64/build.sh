#!/usr/bin/env bash

cmake . \
-DCMAKE_BUILD_TYPE=Release \
-DCMAKE_INSTALL_PREFIX="../../../../target/classes/linux64" \
-G "Unix Makefiles"

make
r1=$?

mkdir -p ../../../../target/classes/linux64/
cp Pxl ../../../../target/classes/linux64/Pxl

exit ${r1}
