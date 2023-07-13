cmake_minimum_required(VERSION 3.8.2)
project(cpp_geodesics)


# Build the geodcircles application that uses VCGLIB and computes geodesic circle stats and/or geodesic mean distances.
set(SOURCE_FILES_COMMON_VCG third_party/vcglib/wrap/ply/plylib.cpp)
set(SOURCE_FILES_GEODCIRCLES_VCG src/geodcircles/main.cpp ${SOURCE_FILES_COMMON_VCG})
add_executable(geodcircles ${SOURCE_FILES_GEODCIRCLES_VCG})
target_include_directories(geodcircles PUBLIC include src/common_vcg)
target_include_directories(geodcircles PUBLIC include src/common)
target_include_directories(geodcircles PUBLIC include third_party/libfs)
target_include_directories(geodcircles PUBLIC include third_party/vcglib)
target_include_directories(geodcircles PUBLIC include third_party/vcglib/eigenlib)
target_include_directories(geodcircles PUBLIC include third_party/spline)
target_include_directories(geodcircles PUBLIC include third_party/tinycolormap)

set_property(TARGET geodcircles PROPERTY CXX_STANDARD 11)
set_property(TARGET geodcircles PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET geodcircles PROPERTY CXX_EXTENSIONS OFF)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(geodcircles PUBLIC OpenMP::OpenMP_CXX)
endif()


if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( geodcircles PRIVATE -Wall -Wextra)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    ADD_DEFINITIONS("_CRT_SECURE_NO_WARNINGS=1") # Disable warnings about fopen, strcpy, etc under msvcc
	target_compile_options( geodcircles PRIVATE /W4 /WX )
endif()


# Build the demo_vcglibbrain application that uses VCGLIB.
set(SOURCE_FILES_DEMO_VCGLIB src/demo_vcglib/main.cpp ${SOURCE_FILES_COMMON_VCG})
add_executable(demo_vcglibbrain ${SOURCE_FILES_DEMO_VCGLIB})
target_include_directories(demo_vcglibbrain PUBLIC include src/common_vcg)
target_include_directories(demo_vcglibbrain PUBLIC include src/common)
target_include_directories(demo_vcglibbrain PUBLIC include third_party/libfs)
target_include_directories(demo_vcglibbrain PUBLIC include third_party/vcglib)
target_include_directories(demo_vcglibbrain PUBLIC include third_party/vcglib/eigenlib)
target_include_directories(demo_vcglibbrain PUBLIC include third_party/spline)
target_include_directories(demo_vcglibbrain PUBLIC include third_party/tinycolormap)

set_property(TARGET demo_vcglibbrain PROPERTY CXX_STANDARD 11)
set_property(TARGET demo_vcglibbrain PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET demo_vcglibbrain PROPERTY CXX_EXTENSIONS OFF)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(demo_vcglibbrain PUBLIC OpenMP::OpenMP_CXX)
endif()


if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( demo_vcglibbrain PRIVATE -Wall -Wextra)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( demo_vcglibbrain PRIVATE /W4 /WX )
endif()


# Build the export_brainmesh application that uses VCGLIB.
set(SOURCE_FILES_EXPORT_BRAINMESH src/export_brainmesh/main.cpp ${SOURCE_FILES_COMMON_VCG})
add_executable(export_brainmesh ${SOURCE_FILES_EXPORT_BRAINMESH})
target_include_directories(export_brainmesh PUBLIC include src/common_vcg)
target_include_directories(export_brainmesh PUBLIC include src/common)
target_include_directories(export_brainmesh PUBLIC include third_party/libfs)
target_include_directories(export_brainmesh PUBLIC include third_party/vcglib)
target_include_directories(export_brainmesh PUBLIC include third_party/vcglib/eigenlib)
target_include_directories(export_brainmesh PUBLIC include third_party/spline)
target_include_directories(export_brainmesh PUBLIC include third_party/tinycolormap)

set_property(TARGET export_brainmesh PROPERTY CXX_STANDARD 11)
set_property(TARGET export_brainmesh PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET export_brainmesh PROPERTY CXX_EXTENSIONS OFF)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(export_brainmesh PUBLIC OpenMP::OpenMP_CXX)
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( export_brainmesh PRIVATE -Wall -Wextra)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( export_brainmesh PRIVATE /W4 /WX )
endif()


# Build the meshneigh_edge application that uses VCGLIB.
set(SOURCE_FILES_MESHNEIGH_EDGE src/meshneigh_edge/main_meshneigh_edge.cpp ${SOURCE_FILES_COMMON_VCG})
add_executable(meshneigh_edge ${SOURCE_FILES_MESHNEIGH_EDGE})
target_include_directories(meshneigh_edge PUBLIC include src/common_vcg)
target_include_directories(meshneigh_edge PUBLIC include src/common)
target_include_directories(meshneigh_edge PUBLIC include third_party/libfs)
target_include_directories(meshneigh_edge PUBLIC include third_party/vcglib)
target_include_directories(meshneigh_edge PUBLIC include third_party/vcglib/eigenlib)
target_include_directories(meshneigh_edge PUBLIC include third_party/spline)

set_property(TARGET meshneigh_edge PROPERTY CXX_STANDARD 11)
set_property(TARGET meshneigh_edge PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET meshneigh_edge PROPERTY CXX_EXTENSIONS OFF)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(meshneigh_edge PUBLIC OpenMP::OpenMP_CXX)
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( meshneigh_edge PRIVATE -Wall -Wextra)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( meshneigh_edge PRIVATE /W4 /WX )
endif()


# Build the meshneigh_geod application that uses VCGLIB.
set(SOURCE_FILES_MESHNEIGH_GEOD src/meshneigh_geod/main_meshneigh_geod.cpp ${SOURCE_FILES_COMMON_VCG})
add_executable(meshneigh_geod ${SOURCE_FILES_MESHNEIGH_GEOD})
target_include_directories(meshneigh_geod PUBLIC include src/common_vcg)
target_include_directories(meshneigh_geod PUBLIC include src/common)
target_include_directories(meshneigh_geod PUBLIC include third_party/libfs)
target_include_directories(meshneigh_geod PUBLIC include third_party/vcglib)
target_include_directories(meshneigh_geod PUBLIC include third_party/vcglib/eigenlib)
target_include_directories(meshneigh_geod PUBLIC include third_party/spline)

set_property(TARGET meshneigh_geod PROPERTY CXX_STANDARD 11)
set_property(TARGET meshneigh_geod PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET meshneigh_geod PROPERTY CXX_EXTENSIONS OFF)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(meshneigh_geod PUBLIC OpenMP::OpenMP_CXX)
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( meshneigh_geod PRIVATE -Wall -Wextra)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( meshneigh_geod PRIVATE /W4 /WX )
endif()



# Build the demo_geolibbrain demo app that uses the the 'geodesic' lib
set(SOURCE_FILES_DEMO_GEOLIB src/demo_geolib/main_geo.cpp)
add_executable(demo_geolibbrain ${SOURCE_FILES_DEMO_GEOLIB})
target_include_directories(demo_geolibbrain PUBLIC include third_party/libfs)
target_include_directories(demo_geolibbrain PUBLIC include third_party/geodesic )

set_property(TARGET demo_geolibbrain PROPERTY CXX_STANDARD 11)
set_property(TARGET demo_geolibbrain PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET demo_geolibbrain PROPERTY CXX_EXTENSIONS OFF)

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( demo_geolibbrain PRIVATE -Wall -Wextra)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( demo_geolibbrain PRIVATE /W4 /WX )
endif()


# Build the geodpath app that uses the the 'geodesic' lib
set(SOURCE_FILES_GEODPATH src/geodpath/main_geodpath.cpp)
add_executable(geodpath ${SOURCE_FILES_GEODPATH})
target_include_directories(geodpath PUBLIC include third_party/libfs)
target_include_directories(geodpath PUBLIC include third_party/geodesic )

set_property(TARGET geodpath PROPERTY CXX_STANDARD 11)
set_property(TARGET geodpath PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET geodpath PROPERTY CXX_EXTENSIONS OFF)


if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( geodpath PRIVATE -Wall -Wextra)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( geodpath PRIVATE /W4 /WX )
endif()


# Build the unit tests, they mainly test functions combining libfs and VCGLIB.
set(SOURCE_FILES_TESTS src/tests/main.cpp src/tests/cpp_geodesic_tests.cpp ${SOURCE_FILES_COMMON_VCG})
add_executable(cpp_geodesic_tests ${SOURCE_FILES_TESTS})
target_include_directories(cpp_geodesic_tests PUBLIC include src/common_vcg)
target_include_directories(cpp_geodesic_tests PUBLIC include src/common)
target_include_directories(cpp_geodesic_tests PUBLIC include third_party/libfs)
target_include_directories(cpp_geodesic_tests PUBLIC include third_party/vcglib)
target_include_directories(cpp_geodesic_tests PUBLIC include third_party/vcglib/eigenlib)
target_include_directories(cpp_geodesic_tests PUBLIC include third_party/spline)
target_include_directories(cpp_geodesic_tests PUBLIC include third_party/catch)

set_property(TARGET cpp_geodesic_tests PROPERTY CXX_STANDARD 11)
set_property(TARGET cpp_geodesic_tests PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET cpp_geodesic_tests PROPERTY CXX_EXTENSIONS OFF)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(cpp_geodesic_tests PUBLIC OpenMP::OpenMP_CXX)
endif()

if( CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU" )
    target_compile_options( cpp_geodesic_tests PRIVATE -Wall -Wextra)
endif()
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options( cpp_geodesic_tests PRIVATE /W4 /WX )
endif()
