cmake_minimum_required( VERSION 2.8 )

project( fontParser )
find_package( Freetype REQUIRED )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )
set( CMAKE_CXX_FLAGS "-std=c++11" )
set( CMAKE_BUILD_TYPE Debug )

file( GLOB all_SRCS
    "${PROJECT_SOURCE_DIR}/include/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.cpp" )

add_executable( ${PROJECT_NAME} ${all_SRCS} )

target_link_libraries( ${PROJECT_NAME} ${FREETYPE_LIBRARIES} )
target_include_directories( ${PROJECT_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS} )
