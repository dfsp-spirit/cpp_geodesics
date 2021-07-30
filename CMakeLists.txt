cmake_minimum_required(VERSION 3.8.2)
project(cpp_geodesics)

# Build the geodesic_vcg application that uses VCGLIB
set(SOURCE_FILES_GEODESIC_VCG src/main.cpp src/typedef_vcg.h third_party/vcglib/wrap/ply/plylib.cpp)
add_executable(geodesics_vcg ${SOURCE_FILES_GEODESIC_VCG})
target_include_directories(geodesics_vcg PUBLIC include third_party/libfs)
target_include_directories(geodesics_vcg PUBLIC include third_party/vcglib )
target_include_directories(geodesics_vcg PUBLIC include third_party/vcglib/eigenlib )
target_include_directories(geodesics_vcg PUBLIC include third_party/spline )

set_property(TARGET geodesics_vcg PROPERTY CXX_STANDARD 11)
set_property(TARGET geodesics_vcg PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET geodesics_vcg PROPERTY CXX_EXTENSIONS OFF)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(geodesics_vcg PUBLIC OpenMP::OpenMP_CXX)
endif()


if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( geodesics_vcg PRIVATE -Wall -Wextra)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( geodesics_vcg PRIVATE /W4 /WX )
endif()


# Build the geodesic_gd app that uses the the 'geodesic' lib
set(SOURCE_FILES_GEODESIC_GEO src/main_geo.cpp)
add_executable(geodesics_geo ${SOURCE_FILES_GEODESIC_GEO})
target_include_directories(geodesics_geo PUBLIC include third_party/libfs)
target_include_directories(geodesics_geo PUBLIC include third_party/geodesic )

set_property(TARGET geodesics_geo PROPERTY CXX_STANDARD 11)
set_property(TARGET geodesics_geo PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET geodesics_geo PROPERTY CXX_EXTENSIONS OFF)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(geodesics_geo PUBLIC OpenMP::OpenMP_CXX)
endif()


if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( geodesics_geo PRIVATE -Wall -Wextra)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( geodesics_geo PRIVATE /W4 /WX )
endif()
