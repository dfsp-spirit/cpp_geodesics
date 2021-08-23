
// The main for the meshneigh_geod program. This main file uses the VCGLIB algorithm.
// The program computes neighborhoods of vertices on meshes and saves them to files.
// The neighborhood is defined by geodesic distance along the mesh.

#include "libfs.h"
#include "typedef_vcg.h"
#include "fs_mesh_to_vcg.h"
#include "mesh_export.h"
#include "mesh_adj.h"
#include "mesh_geodesic.h"
#include "write_data.h"


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>


/// Compute geodesic neighborhood up to max dist for the mesh.
/// @param max_dist float, the distance defining the geodesic neighborhood circle.
void mesh_neigh_geod(const std::string& input_mesh_file, const float max_dist = 5.0, const std::string& output_dist_file="geod_distances", bool include_self = true) {        

    std::cout << "Reading mesh '" + input_mesh_file + "' to compute geodesic distance up to " + std::to_string(max_dist) + " along mesh...\n";
    if(include_self) {
        std::cout << " * Neighborhoods will include the query vertex itself.\n";
    } else {
        std::cout << " * Neighborhoods will NOT include the query vertex itself.\n";
    }
   
    fs::Mesh surface;
    fs::read_surf(&surface, input_mesh_file);

    // Create a VCGLIB mesh from the libfs Mesh.
    std::cout << "Creating VCG mesh from brain surface with " << surface.num_vertices() << " vertices and " << surface.num_faces() << " faces.\n";
    MyMesh m;
    vcgmesh_from_fs_surface(&m, surface);

    // Compute adjacency list representation of mesh
    std::cout << "Computing neighborhoods...\n";
    std::vector<int> query_vertices(surface.num_vertices());
    for(int i=0; i<m.vn; i++) {
        query_vertices[i] = i;
    }
    std::vector<std::vector<GeodNeighbor>> neigh = geod_neighborhood(m, max_dist, include_self);
    
    // Write it to a JSON file.
    //std::string output_dist_file_json = output_dist_file + ".json";
    //strtofile(geod_neigh_to_json(neigh), output_dist_file_json);    
    //std::cout << "Neighborhood information written to file '" + output_dist_file_json + "'.\n";

    // Write it to VV files.
    std::vector<std::vector<int32_t>> neigh_idx(neigh.size(), std::vector<int32_t>());
    std::vector<std::vector<_Float32>> neigh_dist(neigh.size(), std::vector<_Float32>());
    for(size_t i=0; i<neigh.size(); i++) {
        for(size_t j=0; j<neigh[i].size(); j++) {
            neigh_idx[i].push_back(neigh[i][j].index);
            neigh_dist[i].push_back(neigh[i][j].distance);
        }
    }
    
    std::string output_dist_file_index = output_dist_file + "_index.vv";
    std::string output_dist_file_dist = output_dist_file + "_dist.vv";
    write_vv<int32_t>(output_dist_file_index, neigh_idx);
    std::cout << "Geodesic Neighborhood information indices written to file '" + output_dist_file_index + "'.\n";
    write_vv<_Float32>(output_dist_file_index, neigh_dist);
    std::cout << "Geodesic Neighborhood information distances written to file '" + output_dist_file_dist + "'.\n";
}


int main(int argc, char** argv) {
    std::string input_mesh_file;
    std::string output_dist_file = "geod_distances";    
    float max_dist = 5.0;
    bool include_self = true;
    
    if(argc < 2 || argc > 5) {
        std::cout << "===" << argv[0] << " -- Compute geodesic neighborhoods for mesh vertices. ===\n";
        std::cout << "Usage: " << argv[0] << " <input_mesh> [<max_dist> [<output_file> [<include_self>]]]>\n";
        std::cout << "   <input_mesh>    : str, a mesh file in a format supported by libfs, e.g., FreeSurfer, PLY, OBJ, OFF.\n";
        std::cout << "   <max_dist>      : float, the maximal distance to travel along the mesh when defining neighbors. Defaults to 5.0.\n";
        std::cout << "   <output_file>   : str, file name for the output file (suffix gets added, will be overwritten if existing). Default: geod_distances.\n";
        std::cout << "   <include_self>  : bool, whether to include vertex itself in neighborhood, must be 'true' or 'false'. Default: 'true'.\n";
        exit(1);
    }
    input_mesh_file = argv[1];
    if(argc >= 3) {
        std::istringstream iss( argv[2] );        
        if(!(iss >> max_dist)) {
            throw std::runtime_error("Could not convert argument max_dist to float.\n");
        }
        if(max_dist < 0.0) {
            throw std::runtime_error("Value of argument max_dist must not be negative.\n");
        }
    }
    if(argc >= 4) {
        output_dist_file = argv[3];                
    }
    if(argc >= 5) {
        std::string inc = argv[4];
        if(inc == "true") {
            include_self = true;
        } else if(inc == "false") {
            include_self = false;
        } else {
            throw std::runtime_error("Argument include_self must be 'true' or 'false'.\n");
        }
    }        
        
    mesh_neigh_geod(input_mesh_file, max_dist, output_dist_file, include_self);
    exit(0);
}
