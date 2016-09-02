mkdir -p build
cd build
cmake -G "Visual Studio 12 Win64" ..
cmake --build . --config Release
