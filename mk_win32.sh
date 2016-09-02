mkdir -p build32
cd build32
cmake -G "Visual Studio 12" ..
cmake --build . --config Release
echo scp.exe bin/superslew/32/Release/win.xpl pberck@192.168.1.66:dropbox/BIN/superslew/32/
