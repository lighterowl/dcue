call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=MinSizeRel -G Ninja
ninja
7z a -t7z -m0=lzma -mx=9 -mfb=64 -md=32m -ms=on -sse dcue.7z dcue.exe
certutil -hashfile dcue.7z SHA256
