
// The main for the meshneigh_edge program. This main file uses the VCGLIB algorithm.
// The program computes neighborhoods of vertices on meshes and saves them to files.
// The neighborhood is defined by edge distance in the mesh (aka graph distance).

#define APPTAG "[cpp_edge] "
#define CPP_GEOD_DEBUG_LEVEL 4
#include "cppgeod_settings.h"

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
/// @param output_dist_file base name for distance output. will get file extension added.
/// @param include_self bool, whether to include central vertex itself in neighborhood
/// @param with_neigh whether to also write data based on unified Neighborhood format (supported for geodesic and Euclidean distances).
/// @param write_json bool, whether to export in JSON format. not recommended, too bloated.
/// @param write_csv bool, whether to export in CSV format. recommended, large but standard for ML.
/// @param write_vvbin bool, whether to export in custom, binary vvbin format. small but non-standard. easy and quick to parse, but you may need to write loader if reading with another programming language.
/// @param with_neigh bool, whether to write Neighborhood information (as opposed to only distances). Highly recommended.
/// @param input_pvd_file str, path to per-vertex data (like pial lgi, thickness) file for mesh.
/// @param input_ctx_file str, path to cortex label file for mesh to identify cortex versus medial wall vertices and remove the latter.
void mesh_neigh_edge(const std::string& input_mesh_file, const size_t k = 1, const std::string& output_dist_file="edge_distances", const bool include_self=true, const bool write_json=false, const bool write_csv=false, const bool write_vvbin=true, const bool with_neigh=false, const std::string& input_pvd_file="", const std::string& input_ctx_file="") {

    debug_print(CPP_GEOD_DEBUG_LVL_VERBOSE, "Reading mesh '" + input_mesh_file + "' to compute graph " + std::to_string(k) + "-ring edge neighborhoods...");
    if(include_self) {
        debug_print(CPP_GEOD_DEBUG_LVL_INFO, " * Neighborhoods will include the query vertex itself.");
    } else {
        debug_print(CPP_GEOD_DEBUG_LVL_INFO, " * Neighborhoods will NOT include the query vertex itself.");
    }

    fs::Mesh surface;
    fs::read_surf(&surface, input_mesh_file);

    // Create a VCGLIB mesh from the libfs Mesh.
    debug_print(CPP_GEOD_DEBUG_LVL_VERBOSE, "Creating VCG mesh from brain surface with " + std::to_string(surface.num_vertices()) + " vertices and " + std::to_string(surface.num_faces()) + " faces.");
    MyMesh m;
    vcgmesh_from_fs_surface(&m, surface);

    // Compute adjacency list representation of mesh
    debug_print(CPP_GEOD_DEBUG_LVL_INFO, "Computing neighborhoods...");
    std::vector<int> query_vertices(surface.num_vertices());
    for(int i=0; i<m.vn; i++) {
        query_vertices[i] = i;
    }
    std::vector<std::vector<int32_t>> neigh = mesh_adj(m, query_vertices, k, include_self);

    std::vector<Neighborhood> nh;

    const bool write_dists = false;

    const std::string output_neigh_file = output_dist_file + "_neigh";
    if(with_neigh) {
        nh = neighborhoods_from_edge_neighbors(neigh, m);
    }

    std::vector<bool> is_cortex = std::vector<bool>(m.vn, true);
    if(! input_ctx_file.empty()) {
        fs::Label lab;
        fs::read_label(&lab, input_ctx_file);
        is_cortex = lab.vert_in_label(m.vn);
    }

    // Write it to a JSON file.
    if(write_json) {
        if(write_dists) {
            std::string output_dist_file_json = output_dist_file + ".json";
            strtofile(edge_neigh_to_json(neigh), output_dist_file_json);
            std::cout << std::string(APPTAG) << "Neighborhood edge distance information written to JSON file '" + output_dist_file_json + "'.\n";
        }
        if(with_neigh) {
            //std::string output_neigh_file_json = output_neigh_file + ".json";
            //strtofile(neighborhoods_to_json(nh), output_neigh_file_json);
            //std::cout << "Neighborhood information written to JSON file '" + output_neigh_file_json + "'.\n";
            debug_print(CPP_GEOD_DEBUG_LVL_WARN, "Writing Neighborhood information to JSON format not supported yet, skipping. Use CSV instead.");
        }
    }

    // Write it to a VV file.
    if(write_vvbin) {
        if(write_dists) {
            std::string output_dist_file_vv = output_dist_file + ".vv";
            write_vv<int32_t>(output_dist_file_vv, neigh);
            debug_print(CPP_GEOD_DEBUG_LVL_INFO, "Neighborhood information written to vv file '" + output_dist_file_vv + "'.");
        }
        if(with_neigh) {
            std::string output_neigh_file_vv = output_neigh_file + ".vv";
            write_vv<float>(output_neigh_file_vv, neighborhoods_to_vvbin(nh));
            debug_print(CPP_GEOD_DEBUG_LVL_INFO, "Neighborhood information based on Euclidean distance written to vvbin file '" + output_neigh_file_vv + "'.");
        }
    }

    // Write it to a CSV file.
    if(write_csv) {
        if(write_dists) {
            std::string output_dist_file_csv = output_dist_file + ".csv";
            strtofile(edge_neigh_to_csv(neigh), output_dist_file_csv);
            debug_print(CPP_GEOD_DEBUG_LVL_INFO, "Neighborhood edge distance information written to CSV file '" + output_dist_file_csv + "'.");
        }
        if(with_neigh) {
            std::string output_neigh_file_csv = output_neigh_file + ".csv";
            strtofile(neighborhoods_to_csv(nh, 0, false, true, true, input_pvd_file), output_neigh_file_csv);
            debug_print(CPP_GEOD_DEBUG_LVL_INFO, "Neighborhood information based on Euclidean distance written to CSV file '" + output_neigh_file_csv + "'.");
        }
    }
}


int main(int argc, char** argv) {
    std::string input_mesh_file;
    std::string output_dist_file = "edge_distances";
    bool include_self = true;
    bool json = false;
    bool csv = false;
    bool vvbin = true;
    bool with_neigh = false;
    size_t k = 1;
    std::string input_pvd_file = "";
    std::string input_ctx_file = "";

    if(argc < 2 || argc > 11) {
        std::cout << "===" << argv[0] << " -- Compute edge neighborhoods for mesh vertices. ===\n";
        std::cout << "Usage: " << argv[0] << " <input_mesh> [<k> [<output_file] [<include_self> [<json>] [<csv>] [<vv>]]]]>\n";
        std::cout << "   <input_mesh>    : str, a mesh file in a format supported by libfs, e.g., FreeSurfer, PLY, OBJ, OFF.\n";
        std::cout << "   <k>             : int, the k for the k-ring neighborhood computation. Defaults to 1.\n";
        std::cout << "   <output_file>   : str, file name for the output file (suffix gets added, will be overwritten if existing). Default: edge_distances.\n";
        std::cout << "   <include_self>  : bool, whether to include vertex itself in neighborhood, must be 'true' or 'false'. Default: 'true'.\n";
        std::cout << "   <json>          : bool, whether to write JSON output, must be 'true' or 'false'. Default: 'false'.\n";
        std::cout << "   <csv>           : bool, whether to write CSV output, must be 'true' or 'false'. Default: 'false'.\n";
        std::cout << "   <vv>            : bool, whether to write VV output, must be 'true' or 'false'. Default: 'true'.\n";
        std::cout << "   <with_neigh>    : bool, whether to also write unified Neighborhood format files, must be 'true' or 'false'. Default: 'false'.\n";
        std::cout << "   <input_pvd>     : str, a per-vertex value file in a format supported by libfs, e.g., FreeSurfer curv or MGH format. Optional, only used for CSV/vv export.\n";
        std::cout << "   <input_ctx>     : str, a file containing label for the cortex versus non-cortex, e.g., typically 'surf/?h.cortex.label'. Optional, used to filter exported vertices.\n";
        exit(1);
    }
    input_mesh_file = argv[1];

    if(! fs::util::file_exists(input_mesh_file)) {
        std::cerr << "Input mesh file '" << input_mesh_file << "' cannot be read. Exiting.\n";
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
    if(argc >= 5) {
        std::string inc = argv[4];
        if(inc == "true") {
            include_self = true;
        } else if(inc == "false") {
            include_self = false;
        } else {
            throw std::runtime_error("Argument 'include_self' must be 'true' or 'false'.\n");
        }
    }
    if(argc >= 6) {
        std::string jout = argv[5];
        if(jout == "true") {
            json = true;
        } else if(jout == "false") {
            json = false;
        } else {
            throw std::runtime_error("Argument 'json' must be 'true' or 'false'.\n");
        }
    }
    if(argc >= 7) {
        std::string csvout = argv[6];
        if(csvout == "true") {
            csv = true;
        } else if(csvout == "false") {
            csv = false;
        } else {
            throw std::runtime_error("Argument 'csv' must be 'true' or 'false'.\n");
        }
    }
    if(argc >= 8) {
        std::string vvout = argv[7];
        if(vvout == "true") {
            vvbin = true;
        } else if(vvout == "false") {
            vvbin = false;
        } else {
            throw std::runtime_error("Argument 'vv' must be 'true' or 'false'.\n");
        }
    }
    if(argc >= 9) {
        std::string swith_neigh = argv[8];
        if(swith_neigh == "true") {
            with_neigh = true;
        } else if(swith_neigh == "false") {
            with_neigh = false;
        } else {
            throw std::runtime_error("Argument 'with_neigh' must be 'true' or 'false'.\n");
        }
    }
    if(argc >= 10) {
        input_pvd_file = argv[9];
        if(! fs::util::file_exists(input_pvd_file)) {
            std::cerr << "Input per-vertex descriptor file '" << input_pvd_file << "' cannot be read. Exiting.\n";
            exit(1);
        }
    }
    if(argc >= 11) {
        input_ctx_file = argv[9];
        if(! fs::util::file_exists(input_ctx_file)) {
            std::cerr << "Input cortex label file '" << input_ctx_file << "' cannot be read. Exiting.\n";
            exit(1);
        }
    }


    std::cout << std::string(APPTAG) << "base settings: input_mesh_file=" << input_mesh_file << ", input_pvd_file=" << input_pvd_file << ", input_ctx_file=" << input_ctx_file << ", k=" << k << "" << ", include_self=" << include_self << "\n";
    std::cout << std::string(APPTAG) << "output settings: json=" << json << ", csv=" << csv << ", vvbin=" << vvbin << ", with_neigh=" << with_neigh << ", output_dist_file=" << output_dist_file << "\n";

    mesh_neigh_edge(input_mesh_file, k, output_dist_file, include_self, json, csv, vvbin, with_neigh, input_pvd_file, input_ctx_file);
    exit(0);
}
