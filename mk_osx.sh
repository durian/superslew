BUILD="build"
if [ -n "$1" ]
then
    echo "COMPILING FOR XPLANE 9"
    cp CMakeListsXP9.txt CMakeLists.txt
    BUILD="build9"
else
    echo "COMPILING FOR XPLANE 10"
    cp CMakeListsXP10.txt CMakeLists.txt
fi
mkdir -p ${BUILD}
cd ${BUILD}
cmake ..
#the next ''cd ..' was _after_ 'make' earlier
#cd .. 
make
cd ..
#cp bin/mac.xpl /Volumes/Luna/Dropbox/Compiled/groundcontrol/
# Update xplanec version:
echo cp bin/groundcontrol/mac.xpl ~/xplanec/Resources/plugins/groundcontrol/

