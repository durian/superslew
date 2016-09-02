if [ -n "$1" ]
then
    echo "COMPILING FOR XPLANE 9"
    cp CMakeListsXP9.txt CMakeLists.txt
else
    echo "COMPILING FOR XPLANE 10"
    cp CMakeListsXP10.txt CMakeLists.txt
fi
cd build32
cmake -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_C_FLAGS=-m32 ..  -DCMAKE_BUILD_TYPE=RELEASE
make
#
echo scp bin32/lin.xpl.so pberck@192.168.0.24:dropbox/Compiled/drop/32/lin.xpl
