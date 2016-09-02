if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#add_definitions(-DDBG)

set( EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin" )
set( LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin" )

if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
   set( EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin32" )
   set( LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin32" )
endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")


IF( NOT CMAKE_BUILD_TYPE )
	SET( CMAKE_BUILD_TYPE "RelWithDebInfo" )
ENDIF()
message(STATUS "Current build type is : ${CMAKE_BUILD_TYPE}")

add_subdirectory(src)
cmake_minimum_required(VERSION 2.8)
