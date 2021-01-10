#!/usr/bin/env bash

set -e

mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_FIND_LIBRARY_SUFFIXES=.a \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_EXE_LINKER_FLAGS=-static

ninja
bzip2 -9 dcue
sha256sum dcue.bz2
