mkdir -p build32
cd build32
cmake -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_C_FLAGS=-m32 ..  -DCMAKE_BUILD_TYPE=RELEASE
make
echo scp bin/superslew/32/lin.xpl pberck@192.168.1.66:dropbox/BIN/superslew/32/
