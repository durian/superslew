BUILD="build"
mkdir -p ${BUILD}
cd ${BUILD}
cmake ..
make
cd ..
echo cp bin/groundcontrol/mac.xpl ~/xplanec/Resources/plugins/groundcontrol/

