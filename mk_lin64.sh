mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make
echo scp bin/superslew/64/lin.xpl pberck@192.168.1.66:dropbox/BIN/superslew/64/

