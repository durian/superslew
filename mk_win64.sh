mkdir -p build
cd build
cmake -G "Visual Studio 12 Win64" ..
cmake --build . --config Release
echo scp.exe bin/superslew/64/Release/win.xpl pberck@192.168.1.66:dropbox/BIN/superslew/64/
