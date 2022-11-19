call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=%1 -G Ninja -DWITH_TESTS=TRUE
ninja -v

cd ..
build\dcue_gtest.exe
