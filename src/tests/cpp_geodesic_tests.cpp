
// Utility and basic stuff.
#include "libfs.h"
#include "catch.hpp"
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>

// The files including the functions we want to test.
#include "fs_mesh_to_vcg.h"
#include "mesh_edges.h"
#include "mesh_coords.h"
#include "mesh_normals.h"


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

    fs::Mesh surface = fs::Mesh::construct_cube();
    MyMesh m;
    vcgmesh_from_fs_surface(&m, surface);

    std::vector<std::vector<float>> coords = mesh_vertex_coords(m);

    SECTION("The number of vertex coords is correct" ) {
        REQUIRE( int(coords.size()) == m.vn);
    }

    SECTION("The vertex coords are reasonable" ) {
        size_t num_greater_05 = 0;
        size_t num_smaller_minus05 = 0;
        float too_large = 10.0;
        float too_small = -10.0;
        for(size_t i = 0; i < coords.size(); i++) {
            for(size_t j = 0; j < coords[i].size(); j++) {
                if (coords[i][j] > 0.5 && coords[i][j] < too_large) {
                    num_greater_05++;
                }
                if (coords[i][j] < -0.5 && coords[i][j] > too_small) {
                    num_smaller_minus05++;
                }
            }
        }
        REQUIRE( num_greater_05 > 4);
        REQUIRE( num_smaller_minus05 > 4);
    }
}


TEST_CASE( "We can compute the vertex normals of a VCGLIB mesh" ) {

    fs::Mesh surface = fs::Mesh::construct_cube();
    MyMesh m;
    vcgmesh_from_fs_surface(&m, surface);

    std::vector<std::vector<float>> normals = mesh_vnormals(m);

    SECTION("The vertex normals are reasonable" ) {
        size_t num_greater_05 = 0;
        size_t num_smaller_minus05 = 0;
        float too_large = 10.0;
        float too_small = -10.0;
        for(size_t i = 0; i < normals.size(); i++) {
            for(size_t j = 0; j < normals[i].size(); j++) {
                if (normals[i][j] > 0.5 && normals[i][j] < too_large) {
                    num_greater_05++;
                }
                if (normals[i][j] < -0.5  && normals[i][j] > too_small) {
                    num_smaller_minus05++;
                }
            }
        }
        REQUIRE( num_greater_05 > 4);
        REQUIRE( num_smaller_minus05 > 4);

    }
}

