cmake_minimum_required( VERSION 2.8 )

project( GLCore )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib )
set( CMAKE_CXX_FLAGS "-std=c++11 -g" )
set( CMAKE_BUILD_TYPE Debug )

include_directories( ${PROJECT_SOURCE_DIR}/include )

file( GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/include/GL/*.h"
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.src" )

add_library( ${PROJECT_NAME} STATIC ${all_SRCS} )

if (UNIX)
    target_link_libraries( ${PROJECT_NAME} GL GLU X11 xcb X11-xcb png z )
endif (UNIX)

if (WIN32)
    target_link_libraries( ${PROJECT_NAME} opengl32 glu32 m )
endif (WIN32)
