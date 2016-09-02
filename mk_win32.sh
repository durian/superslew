if [ -n "$1" ]
then
    echo "COMPILING FOR XPLANE 9"
    cp CMakeListsXP9.txt CMakeLists.txt
else
    echo "COMPILING FOR XPLANE 10"
    cp CMakeListsXP10.txt CMakeLists.txt
fi
cd build32
cmake -G "Visual Studio 12" ..
cmake --build . --config Release
