call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=%1 -G Ninja
ninja -v
