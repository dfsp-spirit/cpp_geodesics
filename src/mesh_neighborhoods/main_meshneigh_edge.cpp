
// The main for the mesh_neighborhoods program. This main file uses the VCGLIB algorithm.
// The program computes neighborhoods of vertices on meshes and saves them to files.
// A neighborhood can be defined by edge distance in the mesh or by geodesic distance.

#include "libfs.h"
#include "typedef_vcg.h"
#include "fs_mesh_to_vcg.h"
#include "mesh_export.h"
#include "mesh_adj.h"
#include "mesh_geodesic.h"


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>


/// Compute edge neighborhood (or graph k ring) for the mesh.
/// @param k The 'k' for computing the k-ring neighborhood.
void mesh_neigh_edge(const std::string& input_mesh_file, const size_t k = 1, const std::string& output_dist_file="edge_distances.json") {        

    std::cout << " Reading mesh '" + input_mesh_file + "' to compute graph " + std::to_string(k) + "-ring edge neighborhoods...\n";
   
    fs::Mesh surface;
    fs::read_surf(&surface, input_mesh_file);

    // Create a VCGLIB mesh from the libfs Mesh.
    std::cout << " Creating VCG mesh from brain surface with " << surface.num_vertices() << " vertices and " << surface.num_faces() << " faces.\n";
    MyMesh m;
    vcgmesh_from_fs_surface(&m, surface);

    // Compute adjacency list representation of mesh
    std::cout << " Computing adjacency list representation of mesh.\n";
    std::vector<int> query_vertices;
    for(int i=0; i<m.vn; i++) {
        query_vertices.push_back(i);
    }
    std::vector<std::vector<int>> neigh = mesh_adj(m, query_vertices, k, false);
    // Write it to a JSON file.
    strtofile(neigh_to_json(neigh), output_dist_file);
    std::cout << "Neighborhood information written to file '" + output_dist_file + "'.\n";
}


int main(int argc, char** argv) {
    std::string input_mesh_file;
    std::string output_dist_file = "edge_distances.json";
    size_t k = 1;
    
    if(argc < 2 || argc > 4) {
        std::cout << "===" << argv[0] << " -- Compute mesh distances. ===\n";
        std::cout << "Usage: " << argv[0] << " <input_mesh> [<k> [<output_file]]>\n";
        std::cout << "   <input_mesh>    : str, a mesh file in a format supported by libfs, e.g., FreeSurfer, PLY, OBJ, OFF.\n";
        std::cout << "   <k>             : int, the k for the k-ring neighborhood computation. Defaults to 1.\n";
        std::cout << "   <output_file>   : str, file name for the output JSON file (will be overwritten if existing). Default: edge_distances.json.\n";
        exit(1);
    }
    if(argc >= 3) {
        std::istringstream iss( argv[2] );        
        if(!(iss >> k)) {
            throw std::runtime_error("Could not convert argument k to positive integer.\n");
        }        
    }
    if(argc >= 4) {
        output_dist_file = argv[3];                
    }
        
    mesh_neigh_edge(input_mesh_file, k, output_dist_file);
    exit(0);
}
