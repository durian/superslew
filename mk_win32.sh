mkdir -p build32
cd build32
cmake -G "Visual Studio 12" ..
cmake --build . --config Release
