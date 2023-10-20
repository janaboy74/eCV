cmake_minimum_required(VERSION 2.8.12...3.13)

add_subdirectory( libGLCore )

project( eCV )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )
set( CMAKE_CXX_FLAGS "-std=c++11 -g" )
set( CMAKE_BUILD_TYPE Debug )

include_directories( ${PROJECT_SOURCE_DIR}/libGLCore/include )
include_directories( ${PROJECT_SOURCE_DIR}/include )
link_directories( ${PROJECT_SOURCE_DIR}/libGLCore/lib )

file( GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.cpp" )

add_executable( ${PROJECT_NAME} ${all_SRCS} )

target_link_libraries( ${PROJECT_NAME} GLCore )

if (UNIX)
    target_link_libraries( ${PROJECT_NAME} GL GLU X11 xcb X11-xcb pthread )
endif (UNIX)

if (WIN32)
    target_link_libraries( ${PROJECT_NAME} opengl32 glu32 m )
endif (WIN32)
