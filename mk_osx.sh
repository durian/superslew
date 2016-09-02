BUILD="build"
mkdir -p ${BUILD}
cd ${BUILD}
cmake ..
make
cd ..
echo cp bin/superslew/mac.xpl ~/xplanec/Resources/plugins/superslew/

