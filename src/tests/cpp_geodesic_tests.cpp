
#include "libfs.h"
#include "catch.hpp"
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>


TEST_CASE( "Reading the demo cube mesh file with read_mesh works" ) {

    fs::Mesh surface;
    fs::read_mesh(&surface, "demo_data/meshes/cube.ply");

    SECTION("The number of vertices read is correct" ) {
        REQUIRE( surface.num_vertices() == 8);
    }
}
