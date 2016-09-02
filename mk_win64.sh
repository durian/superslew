cd build
cmake -G "Visual Studio 12 Win64" ..
cmake --build . --config Release

#scp.exe bin/Release/win.xpl.dll pberck@192.168.0.24:dropbox/Compiled/drop/64/
#scp.exe bin32/Release/win.xpl.dll pberck@192.168.0.24:dropbox/Compiled/drop/32/win.xpl
#
echo 'cp bin/Release/win.xpl.dll ~/Desktop/X-Plane\ 10/Resources/plugins/drop/64/win.xpl'
