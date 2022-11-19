#!/usr/bin/env bash

set -e

btype=$1
mkdir "build_${btype}_${CXX}"
cd "build_${btype}_${CXX}"
cmake .. -G Ninja "-DCMAKE_BUILD_TYPE=${btype}" \
  "-DCMAKE_CXX_FLAGS=-Wall -Wextra -Werror" \
  -DWITH_TESTS=TRUE
ninja -v

./dcue_gtest

exit 0
