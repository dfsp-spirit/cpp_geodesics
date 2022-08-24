
// The main for the meshneigh_edge program. This main file uses the VCGLIB algorithm.
// The program computes neighborhoods of vertices on meshes and saves them to files.
// The neighborhood is defined by edge distance in the mesh (aka graph distance).

#include "libfs.h"
#include "typedef_vcg.h"
#include "fs_mesh_to_vcg.h"
#include "mesh_export.h"
#include "mesh_adj.h"
#include "mesh_geodesic.h"
#include "mesh_neighborhood.h"
#include "write_data.h"


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>


/// @brief Compute edge neighborhood (or graph k ring) for the mesh.
/// @param input_mesh_file input mesh file path
/// @param k The 'k' for computing the k-ring neighborhood. Use 1 for direct edge neighbors.
void mesh_neigh_edge(const std::string& input_mesh_file, const size_t k = 1, const std::string& output_dist_file="edge_distances", const bool include_self=true, const bool write_json=false, const bool write_csv=false, const bool write_vvbin=true) {

    std::cout << "Reading mesh '" + input_mesh_file + "' to compute graph " + std::to_string(k) + "-ring edge neighborhoods...\n";
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
    std::vector<std::vector<int32_t>> neigh = mesh_adj(m, query_vertices, k, include_self);

    std::vector<Neighborhood> nh = neighborhoods_from_edge_neighbors(neigh, m);
    std::cout << "TODO: implement to_csv method for std::vector<Neighborhood> and write 'nh' to files as well.\n";

    const bool write_dists = true;
    const bool write_neigh = false; // Whether to write neighborhoods in addition to adjacency
    const std::string output_neigh_file = "neighborhoods_edge";

    // Write it to a JSON file.
    if(write_json) {
        if(write_dists) {
            std::string output_dist_file_json = output_dist_file + ".json";
            strtofile(edge_neigh_to_json(neigh), output_dist_file_json);
            std::cout << "Neighborhood edge distance information written to JSON file '" + output_dist_file_json + "'.\n";
        }
        if(write_neigh) {
            std::string output_neigh_file_json = output_neigh_file + ".json";
            strtofile(neighborhoods_to_json(nh), output_neigh_file_json);
            std::cout << "Neighborhood information written to JSON file '" + output_neigh_file_json + "'.\n";
        }
    }

    // Write it to a VV file.
    if(write_vvbin) {
        std::string output_dist_file_vv = output_dist_file + ".vv";
        write_vv<int32_t>(output_dist_file_vv, neigh);
        std::cout << "Neighborhood information written to vv file '" + output_dist_file_vv + "'.\n";
    }

    // Write it to a CSV file.
    if(write_csv) {
        std::string output_dist_file_csv = output_dist_file + ".csv";
        strtofile(edge_neigh_to_csv(neigh), output_dist_file_csv);
        std::cout << "Neighborhood information written to CSV file '" + output_dist_file_csv + "'.\n";
    }
}


int main(int argc, char** argv) {
    std::string input_mesh_file;
    std::string output_dist_file = "edge_distances";
    bool include_self = true;
    bool json = false;
    bool csv = false;
    bool vvbin = true;
    size_t k = 1;

    if(argc < 2 || argc > 8) {
        std::cout << "===" << argv[0] << " -- Compute edge neighborhoods for mesh vertices. ===\n";
        std::cout << "Usage: " << argv[0] << " <input_mesh> [<k> [<output_file] [<include_self> [<json>] [<csv>] [<vv>]]]]>\n";
        std::cout << "   <input_mesh>    : str, a mesh file in a format supported by libfs, e.g., FreeSurfer, PLY, OBJ, OFF.\n";
        std::cout << "   <k>             : int, the k for the k-ring neighborhood computation. Defaults to 1.\n";
        std::cout << "   <output_file>   : str, file name for the output file (suffix gets added, will be overwritten if existing). Default: edge_distances.\n";
        std::cout << "   <include_self>  : bool, whether to include vertex itself in neighborhood, must be 'true' or 'false'. Default: 'true'.\n";
        std::cout << "   <json>          : bool, whether to write JSON output, must be 'true' or 'false'. Default: 'false'.\n";
        std::cout << "   <csv>           : bool, whether to write CSV output, must be 'true' or 'false'. Default: 'false'.\n";
        std::cout << "   <vv>            : bool, whether to write VV output, must be 'true' or 'false'. Default: 'true'.\n";
        exit(1);
    }
    input_mesh_file = argv[1];
    if(argc >= 3) {
        std::istringstream iss( argv[2] );
        if(!(iss >> k)) {
            throw std::runtime_error("Could not convert argument k to positive integer.\n");
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
    if(argc >= 6) {
        std::string jout = argv[5];
        if(jout == "true") {
            json = true;
        } else if(jout == "false") {
            json = false;
        } else {
            throw std::runtime_error("Argument json must be 'true' or 'false'.\n");
        }
    }
    if(argc >= 7) {
        std::string csvout = argv[6];
        if(csvout == "true") {
            csv = true;
        } else if(csvout == "false") {
            csv = false;
        } else {
            throw std::runtime_error("Argument csv must be 'true' or 'false'.\n");
        }
    }
    if(argc >= 8) {
        std::string vvout = argv[7];
        if(vvout == "true") {
            vvbin = true;
        } else if(vvout == "false") {
            vvbin = false;
        } else {
            throw std::runtime_error("Argument vv must be 'true' or 'false'.\n");
        }
    }

    mesh_neigh_edge(input_mesh_file, k, output_dist_file, include_self, json, csv, vvbin);
    exit(0);
}
