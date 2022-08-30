
#include "libfs.h"
#include "catch.hpp"
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>

#include "fs_mesh_to_vcg.h"
#include "mesh_edges.h"


TEST_CASE( "Reading the demo cube mesh file with read_mesh works" ) {

    fs::Mesh surface;
    fs::read_mesh(&surface, "demo_data/meshes/cube.ply");

    SECTION("The number of vertices read is correct" ) {
        REQUIRE( surface.num_vertices() == 8);
    }
}


TEST_CASE( "We can compute the edge length of a mesh with VCGLIB" ) {

    fs::Mesh surface = fs::Mesh::construct_pyramid();
    MyMesh m;
    vcgmesh_from_fs_surface(&m, surface);

    std::vector<double> edge_lengths = mesh_edge_lengths(m);

    SECTION("The number of edges is correct" ) {
        REQUIRE( edge_lengths.size() == 9);
    }
}

