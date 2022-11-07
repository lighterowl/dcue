#!/usr/bin/env bash

set -e

mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=MinSizeRel

ninja

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage

VERSION= ./linuxdeploy-x86_64.AppImage -e dcue --create-desktop-file \
  -i ../.github/dcue.png --appdir AppDir --output appimage

mv dcue--x86_64.AppImage dcue-lnx-x86_64.AppImage
sha256sum dcue-lnx-x86_64.AppImage
