
// The main for our code.

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

int main(int argc, char** argv) {
    if(argc > 1) {
        std::cout << "Usage: " << argv[0] << "\n";
        exit(1);
    }

    std::string subject = "fsaverage3";
    std::cout << " Reading FreeSurfer surfaces and labels for subject '" + subject + "'...\n";

    
    std::string lh_surf_file = "demo_data/subjects_dir/" + subject + "/surf/lh.white";
    std::string rh_surf_file = "demo_data/subjects_dir/" + subject + "/surf/rh.white";
    std::string lh_label_file = "demo_data/subjects_dir/" + subject + "/label/lh.cortex.label";
    std::string rh_label_file = "demo_data/subjects_dir/" + subject + "/label/rh.cortex.label";
    std::string lh_curv_file = "demo_data/subjects_dir/" + subject + "/surf/lh.thickness";
    std::string rh_curv_file = "demo_data/subjects_dir/" + subject + "/surf/rh.thickness";

    fs::Mesh lh_white, rh_white; 
    fs::read_surf(&lh_white, lh_surf_file);
    fs::read_surf(&rh_white, rh_surf_file);

    fs::Label lh_cortex, rh_cortex;
    fs::read_label(&lh_cortex, lh_label_file);
    fs::read_label(&rh_cortex, rh_label_file);

    std::vector<float> lh_thickness, rh_thickness;
    lh_thickness = fs::read_curv_data(lh_curv_file);
    rh_thickness = fs::read_curv_data(rh_curv_file);

    // Create a VCGLIB mesh from the libfs Mesh.
    std::cout << " Creating VCG mesh from brain surface.\n";
    MyMesh m;
    vcgmesh_from_fs_surface(&m, lh_white);

    // Export the mesh to PLY format to check that is looks correct (e.g., in Blender).
    std::cout << " Exporting mesh in PLY format to file 'mesh.ply'.\n";
    export_mesh_ply(m, "mesh.ply");

    // Compute adjacency list representation of mesh
    std::cout << " Computing adjacency list representation of mesh.\n";
    std::vector<int> query_vertices;
    for(int i=0; i<m.vn; i++) {
        query_vertices.push_back(i);
    }
    int k = 1;  // The 'k' for computing the k-ring neighborhood.
    std::vector<std::vector<int>> neigh = mesh_adj(m, query_vertices, k, false);
    // Write it to a JSON file.
    strtofile(neigh_to_json(neigh), "mesh_adj.json");
    
    // Compute geodesic distances to a single query vertex
    std::vector<int> query_vertices_geod;
    int qv = 500;
    query_vertices_geod.push_back(qv);
    std::cout << " Computing geodesic distance from query vertex " << qv << " to all others.\n";
    float max_dist = -1.0; // Negative value means no max distance.
    std::vector<float> dists_to_vert = geodist(m, query_vertices_geod, max_dist);

    // Compute mean geodesic distance from each vertex to all others
    std::cout << " Computing mean geodesic distance from each vertex to all others.\n";
    std::vector<float> mean_dists = mean_geodist(m);


    exit(0);
}
