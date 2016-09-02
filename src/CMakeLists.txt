
SET(SRC dataref.cpp Global.cpp Log.cpp sha256.cpp lodepng.cpp main.cpp)

SET(HDR dataref.h Global.h Log.h sha256.h lodepng.h float_window.h main.h)

SET(ALL_SRC ${SRC} ${HDR})
LIST(SORT ALL_SRC)

if(APPLE)
	add_executable(plugin ${ALL_SRC})
	FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation )
	find_package(OpenGL REQUIRED)
	include_directories(${OPENGL_INCLUDE_DIR})
	include_directories(inc)
elseif(UNIX)
	add_library(plugin SHARED ${ALL_SRC})
	if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	   	message("64 bits")
		set(BITS "64")
		find_package(OpenGL REQUIRED)
		include_directories(${OPENGL_INCLUDE_DIR})
		#find_package(GLUT REQUIRED)
		#include_directories(${GLUT_INCLUDE_DIR})
	else()
		message("32 bits")
		set(BITS "32")
		find_package(OpenGL REQUIRED)
		include_directories(${OPENGL_INCLUDE_DIR})
		#find_package(GLUT REQUIRED)
		#include_directories(${GLUT_INCLUDE_DIR})
	endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
elseif(WIN32)
	add_library(plugin MODULE ${ALL_SRC})
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} /MT")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	find_package(OpenGL REQUIRED)
	include_directories(${OPENGL_INCLUDE_DIR})
	#find_package(GLUT REQUIRED)
	#include_directories(${GLUT_INCLUDE_DIR})
endif()

 
#compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11") #-Wall
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCHECK_RESULT_USED=_Check_return_")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCHECK_RESULT_USED=\"__attribute__ ((warn_unused_result))\"")
endif()

#libraries
if(UNIX)
	target_include_directories(plugin PUBLIC "../../SDK/CHeaders/XPLM/" "../../SDK/CHeaders/Widgets/" "../../SDK/CHeaders/Wrappers/")
endif(UNIX)
if(APPLE)
	set (CMAKE_OSX_ARCHITECTURES "i386;x86_64" CACHE STRING "Build architectures for OSX" FORCE)	
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64")
	#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")
endif(APPLE)
if(WIN32)
	target_include_directories(plugin SYSTEM PUBLIC "..\\..\\SDK\\CHeaders\\XPLM" "..\\..\\SDK\\CHeaders\\Widgets" "..\\..\\SDK\\CHeaders\\Wrappers")
endif(WIN32)

# X-Plane stuff
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DXPLM200=1 -DXPLM210=0")
if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAPL=1 -DIBM=0 -DLIN=0")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAPL=0 -DIBM=0 -DLIN=1")
elseif(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DAPL=0 -DIBM=1 -DLIN=0 -D_CRT_SECURE_NO_WARNINGS /bigobj")
endif()

#linking
#link_libraries(m)
if(APPLE)
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ../../SDK/Libraries/Mac/)
	FIND_LIBRARY(XPLM_LIBRARY XPLM ../../../SDK/Libraries/Mac/)
	FIND_LIBRARY(XPWIDGETS_LIBRARY XPWidgets ../../../SDK/Libraries/Mac/)
	target_link_libraries(plugin ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY} ${COREFOUNDATION_LIBRARY} ${OPENGL_LIBRARIES}) # ${GLUT_LIBRARIES})
elseif(UNIX)
	#target_link_libraries(plugin "-lGL" "-lGLU")
	target_link_libraries(plugin ${OPENGL_LIBRARIES}) #"-lGL" "-lGLU")
elseif(WIN32)
	# the next 8 lines not necessary? The 2 after are only 64 bits?
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "..\\..\\SDK\\Libraries\\Win\\")
	if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		FIND_LIBRARY(XPLM_LIBRARY XPLM_64 ..\\..\\SDK\\Libraries\\Win\\)
		FIND_LIBRARY(XPWIDGETS_LIBRARY XPWidgets_64 ..\\..\\SDK\\Libraries\\Win\\)
	else ()
		FIND_LIBRARY(XPLM_LIBRARY XPLM ..\\..\\SDK\\Libraries\\Win\\)
		FIND_LIBRARY(XPWIDGETS_LIBRARY XPWidgets ..\\..\\SDK\\Libraries\\Win\\)
	endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	target_link_libraries(plugin ${XPLM_LIBRARY} ${XPWIDGETS_LIBRARY} ${OPENGL_LIBRARIES})
endif()

#output
SET_TARGET_PROPERTIES(plugin PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(plugin PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(plugin PROPERTIES SUFFIX "")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY${CMAKE_BINARY_DIR}/bin)
		
if(APPLE)
	set(PLUGIN_OUTPUT_FILENAME "mac.xpl")
	SET_TARGET_PROPERTIES(plugin PROPERTIES LINK_FLAGS
	"${CMAKE_SHARED_LINKER_FLAGS}  -fPIC -fvisibility=hidden -bundle -framework OpenGL -framework CoreFoundation")
	set_target_properties(plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/groundcontrol" )
	set_target_properties(plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/groundcontrol" )
	set_target_properties(plugin PROPERTIES OUTPUT_NAME "${PLUGIN_OUTPUT_FILENAME}")
elseif(UNIX)
	set(PLUGIN_OUTPUT_FILENAME "lin.xpl")
	SET_TARGET_PROPERTIES(plugin PROPERTIES LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic -nodefaultlibs -undefined_warning -fPIC -fvisibility=hidden")
	if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	   	set(PLUGIN_OUTPUT_FILENAME "lin.xpl")
		set_target_properties(plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/groundcontrol/64" )
		set_target_properties(plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/groundcontrol/64" )
		set_target_properties(plugin PROPERTIES OUTPUT_NAME "${PLUGIN_OUTPUT_FILENAME}")
	else()
	   	set(PLUGIN_OUTPUT_FILENAME "lin.xpl")
		set_target_properties(plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/groundcontrol/32" )
		set_target_properties(plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/groundcontrol/32" )
		set_target_properties(plugin PROPERTIES OUTPUT_NAME "${PLUGIN_OUTPUT_FILENAME}")
	endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
elseif(WIN32)
	set(PLUGIN_OUTPUT_FILENAME "win.xpl")
	if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	   	set(PLUGIN_OUTPUT_FILENAME "win.xpl")
		set_target_properties(plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/groundcontrol/64" )
		set_target_properties(plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/groundcontrol/64" )
		set_target_properties(plugin PROPERTIES OUTPUT_NAME "${PLUGIN_OUTPUT_FILENAME}")
	else()
	   	set(PLUGIN_OUTPUT_FILENAME "win.xpl")
		set_target_properties(plugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/groundcontrol/32" )
		set_target_properties(plugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/groundcontrol/32" )
		set_target_properties(plugin PROPERTIES OUTPUT_NAME "${PLUGIN_OUTPUT_FILENAME}")
	endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")

endif()

cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
set_property(TARGET plugin PROPERTY CXX_STANDARD 11)
set_property(TARGET plugin PROPERTY CXX_STANDARD_REQUIRED ON)

# Linux:
# cmake -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_C_FLAGS=-m32 ..
# cmake .. -DCMAKE_BUILD_TYPE=RELEASE
# (strip ../bin/lin.xpl.so)
# scp ../bin/lin.xpl.so pberck@192.168.0.24:/Volumes/Luna/SharedVB/gpxlog/64/lin.xpl
#
# but clean out build dir first!
#
# Win
# In dosprompt:
# cmake -G "Visual Studio 12 Win64" ..
#                            ^^^^^ without for 32 bits (use build32 folder)
# cmake --build . target ALL_BUILD
#or
# C:\Windows\Microsoft\Microsoft.NET\Framework\v4.030319\MSBuild src/plugin.vcxproj /p:Configuration=Release
