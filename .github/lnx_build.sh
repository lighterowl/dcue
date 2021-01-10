#!/usr/bin/env bash

set -e

mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

ninja
bzip2 -9 dcue
sha256sum dcue.bz2
