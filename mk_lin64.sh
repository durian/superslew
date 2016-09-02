cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
make
#
echo scp bin/lin.xpl.so pberck@192.168.0.24:dropbox/Compiled/drop/64/lin.xpl
echo cp bin/lin.xpl.so ~/xplane/Resources/plugins/drop/64/lin.xpl
