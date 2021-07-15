cmake_minimum_required(VERSION 3.8.2)
project(libfs)

set(SOURCE_FILES src/main.cpp src/typedef_vcg.h)
include_directories( include third_party/libfs )
include_directories( include third_party/vcglib )
include_directories( include third_party/vcglib/eigenlib )
add_executable(geodesics ${SOURCE_FILES})

set_property(TARGET geodesics PROPERTY CXX_STANDARD 11)
set_property(TARGET geodesics PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET geodesics PROPERTY CXX_EXTENSIONS OFF)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(geodesics PUBLIC OpenMP::OpenMP_CXX)
endif()


if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( geodesics PRIVATE -Wall -Wextra -Werror )
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( geodesics PRIVATE /W4 /WX )
endif()
