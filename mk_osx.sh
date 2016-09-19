BUILD="build"
mkdir -p ${BUILD}
cd ${BUILD}
#https://github.com/Homebrew/legacy-homebrew/issues/46362
cmake -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk ..
make
cd ..
echo cp bin/superslew/mac.xpl ~/xplanec/Resources/plugins/superslew/

