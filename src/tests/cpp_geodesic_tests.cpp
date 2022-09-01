
#include "libfs.h"
#include "catch.hpp"
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>

#include "fs_mesh_to_vcg.h"
#include "mesh_edges.h"
#include "mesh_coords.h"


TEST_CASE( "Reading the demo cube mesh file with read_mesh works" ) {

    fs::Mesh surface;
    fs::read_mesh(&surface, "demo_data/meshes/cube.ply");

    SECTION("The number of vertices read is correct" ) {
        REQUIRE( surface.num_vertices() == 8);
    }

    SECTION("The number of faces read is correct" ) {
        REQUIRE( surface.num_faces() == 12);
    }

    SECTION("The first face is correct" ) {
        REQUIRE( surface.fm_at(0, 0) == 0);  // Known from cube.ply
        REQUIRE( surface.fm_at(0, 1) == 2);
        REQUIRE( surface.fm_at(0, 2) == 3);
    }

    SECTION("The last face is correct" ) {
        REQUIRE( surface.fm_at(11, 0) == 7);  // Known from cube.ply
        REQUIRE( surface.fm_at(11, 1) == 3);
        REQUIRE( surface.fm_at(11, 2) == 1);
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

    SECTION("The edge lengths are reasonable" ) {
        for(size_t i = 0; i < edge_lengths.size(); i++) {
            REQUIRE( edge_lengths[i] < 2.0);
            REQUIRE( edge_lengths[i] > 0.1);
        }
    }


}


TEST_CASE( "We can compute the coordinates of a VCGLIB mesh" ) {

    fs::Mesh surface = fs::Mesh::construct_pyramid();
    MyMesh m;
    vcgmesh_from_fs_surface(&m, surface);

    std::vector<double> edge_lengths = mesh_edge_lengths(m);

    SECTION("The number of edges is correct" ) {
        REQUIRE( edge_lengths.size() == 9);
    }
}

