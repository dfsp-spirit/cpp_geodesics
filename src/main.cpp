
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



/// exec_mode must be 's' or 'p'.
int test_stuff(const std::string& exec_mode) {        

    std::string subject = "fsaverage3";
    //std::string subject = "subject1";
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
    std::cout << " Creating VCG mesh from brain surface with " << lh_white.num_vertices() << " vertices and " << lh_white.num_faces() << " faces.\n";
    MyMesh m;
    vcgmesh_from_fs_surface(&m, lh_white);

    // Test the reverse conversion
    fs::Mesh lh_white2;
    fs_surface_from_vcgmesh(&lh_white2, m);
    std::cout << " Re-created fs::Mesh from VCG mesh, it has " << lh_white2.num_vertices() << " vertices and " << lh_white2.num_faces() << " faces.\n";

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
    std::vector<float> mean_dists;
    std::string mean_geodist_outfile;
    if (exec_mode == "s") {
        std::cout << "Sequential!\n";
        mean_dists = mean_geodist(m);
        mean_geodist_outfile = "geodist_seq.curv";
        fs::write_curv(mean_geodist_outfile, mean_dists);
    } else {
        std::cout << "Parallel!\n";
        mean_dists = mean_geodist_p(m);
        mean_geodist_outfile = "geodist_par.curv";
        fs::write_curv(mean_geodist_outfile, mean_dists);
    }


    // Compute geodesic circle stats
    std::vector<int32_t> qv_cs;
    bool do_meandists = false;
    std::vector<std::vector<float>> circle_stats = geodesic_circles(m, qv_cs, 5.0, do_meandists);
    std::vector<float> radii = circle_stats[0];    
    std::vector<float> perimeters = circle_stats[1];
    std::string rad_filename = "lh." + subject + "_radius_s5.curv";
    std::string per_filename = "lh." + subject + "_perimeter_s5.curv";
    std::string mgd_filename = "lh." + subject + "_meangeodist_geocircles.curv";
    fs::write_curv(rad_filename, radii);
    fs::write_curv(per_filename, perimeters);
    if(do_meandists) {
        std::vector<float> mean_geodists_circ = circle_stats[2]; // Should be identical to the ones in 'mean_geodist' computed above.
        fs::write_curv(mgd_filename, mean_geodists_circ);
    }
    
    return 0;
}

int main(int argc, char** argv) {

    
    //if(argc != 2) {
    //    std::cout << "Usage: " << argv[0] << " s|p\n";
    //    exit(1);
    //}
    //test_stuff(argv[1]);
    //exit(0);

    if(argc < 2 || argc > 3) {
        std::cout << "== Compute mean geodesic distances for each vertex to all others for ?h.pialsurface6 ==.\n";
        std::cout << "Usage: " << argv[0] << " <subjects_file> [<subjects_dir>]\n";
        std::cout << "  <subjects_file> : text file containing one subject identifier per line.\n";
        std::cout << "  <subjects_dir>  : directory containing the FreeSurfer recon-all output for the subjects. Defaults to current working directory.\n";
        exit(1);
    }
    std::string subjects_file = std::string(argv[1]);
    std::string subjects_dir = "./";
    if(argc == 3) {
        subjects_dir = std::string(argv[2]);
    }


    std::vector<std::string> subjects = fs::read_subjectsfile(subjects_file);
    std::cout << "Found " << subjects.size() << " subjects listed in subjects file '" << subjects_file << "'.\n";
    std::cout << "Using subject directory '" << subjects_dir << "'.\n";
    
    const std::vector<std::string> hemis = {"lh", "rh"};
    std::string surf_file, hemi, subject;
    fs::Mesh surface;

    for (size_t i=0; i<subjects.size(); i++) {
        subject = subjects[i];
        std::cout << " * Handling subject '" << subject << "', # " << (i+1) << " of " << subjects.size() << ".\n";
        for (size_t hemi_idx=0; hemi_idx<hemis.size(); hemi_idx++) {
            hemi = hemis[hemi_idx];
            
            // Load FreeSurfer mesh from file.
            surf_file = "./" + subject + "/surf/" + hemi + ".pialsurface6";
            fs::read_surf(&surface, surf_file);

            // Create a VCGLIB mesh from the libfs Mesh.            
            MyMesh m;
            vcgmesh_from_fs_surface(&m, surface);

            // Compute the geodesic mean distances and write result file.
            std::vector<float> mean_dists = mean_geodist_p(m);
            std::string mean_geodist_outfile = "./" + subject + "/surf/" + hemi + ".mean_geodist";
            fs::write_curv(mean_geodist_outfile, mean_dists);
        }
    }

}
