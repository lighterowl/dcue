call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
vcpkg install curl:x86-windows-static
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=MinSizeRel -DVCPKG_TARGET_TRIPLET=x86-windows-static -G Ninja
ninja
7z a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on -sse dcue.7z dcue.exe