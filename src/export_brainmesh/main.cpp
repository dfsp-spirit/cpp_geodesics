
// The main for the export_brainmesh program. This main file uses VCGLIB.
// This application

#include "libfs.h"
#include "typedef_vcg.h"
#include "fs_mesh_to_vcg.h"
#include "mesh_export.h"
#include "mesh_adj.h"
#include "mesh_geodesic.h"
#include "values_to_color.h"


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>


/// Export a colored PLY brain mesh, can be viewed in Meshlab.
void export_brain(const std::string& surf_file, const std::string& curv_file, const std::string& output_ply_file) {
    // Load mesh and data.
    fs::Mesh surface;
    fs::read_mesh(&surface, surf_file);
    std::vector<float> morph_data = fs::read_curv_data(curv_file);

    // Map data to colors
    std::vector<u_int8_t> colors = data_to_colors(morph_data);
    surface.to_ply_file(output_ply_file, colors);
    std::cout << "Vertex-colored brain mesh written to file '" << output_ply_file << "'.\n";
}


/// Export a non-colored or plain brain mesh.
void export_brain(const std::string& surf_file, const std::string& output_ply_file) {
    // Load mesh and data.
    fs::Mesh surface;
    fs::read_mesh(&surface, surf_file);

    // Map data to colors
    surface.to_ply_file(output_ply_file);
    std::cout << "Plain brain mesh written to file '" << output_ply_file << "'.\n";
}

/// Generate unit cube.
/// @param mesh fslib mesh, an empty mesh instance to which to add cube vertices and faces.


int main(int argc, char** argv) {

    if(argc < 3 || argc > 4) {
        std::cout << "== Export colored brain mesh ==.\n";
        std::cout << "Usage: " << argv[0] << "[<surf_file> [<curv_file>] <output_ply_file>] | [--gen-cube <output_ply_file>]\n";
        std::cout << "  <surf_file>       : path to a brain mesh file, typically in FreeSurfer surf format.\n";
        std::cout << "  <curv_file>       : optional, path to a file containing per-vertex data for the mesh, typically in FreeSurfer curv format. If omitted, no colors will be produced.\n";
        std::cout << "  <output_ply_file> : path to the output file in PLY format, will be created (or overwritten in case it exists).\n";
        std::cout << "  Examples: " << argv[0] << " demo_data/subjects_dir/subject1/surf/lh.white demo_data/subjects_dir/subject1/surf/lh.thickness colored_brain.ply\n";
        std::cout << "            " << argv[0] << " demo_data/subjects_dir/subject1/surf/lh.white plain_brain.ply\n";
        std::cout << "            " << argv[0] << " --gen-cube cube_mesh.ply\n";
        std::cout << "Hint: A great software to visualize colored PLY meshes is MeshLab. Run `meshlab mymesh.ply` to view if you have it installed.\n";
        exit(1);
    } else if (argc == 3) {
        if(std::string(argv[1]) == "--gen-cube") {
            std::cout << "Generating simple cube mesh in PLY format and writing to '" << argv[2] << "'\n.";
            fs::Mesh surface = fs::Mesh::construct_cube();
            surface.to_ply_file(argv[2]);
        } else {
            export_brain(argv[1], argv[2]);
        }
    } else { // argc = 4
        export_brain(argv[1], argv[2], argv[3]);
    }
    exit(0);
}
