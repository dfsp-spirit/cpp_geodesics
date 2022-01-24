
// The main for the geodcircles program. This main file uses the VCGLIB algorithm.

#include "libfs.h"
#include "typedef_vcg.h"
#include "fs_mesh_to_vcg.h"
#include "mesh_export.h"
#include "mesh_adj.h"
#include "mesh_geodesic.h"
#include "values_to_color.h"
#include "io.h"


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>


int main(int argc, char** argv) {

    if(argc < 2 || argc > 6) {
        std::cout << "== Compute mean geodesic distances and circle stats for FreeSurfer brain meshes ==.\n";
        std::cout << "Usage: " << argv[0] << " <subjects_file> [<subjects_dir> [<surface> [<do_circle_stats>]]]\n";
        std::cout << "  <subjects_file> : text file containing one subject identifier per line.\n";
        std::cout << "  <subjects_dir>  : directory containing the FreeSurfer recon-all output for the subjects. Defaults to current working directory.\n";
        std::cout << "  <surface>       : the surface file to load from the surf/ subdir of each subject, without hemi part. Defaults to 'pial'.\n";
        std::cout << "  <do_circle_stat>: flag whether to compute geodesic circle stats as well, must be 0 (off), 1 (on) or 2 (on with mean dists). Defaults to 0.\n";
        std::cout << "  <keep_existing> : flag whether to keep existing output files, must be 0 (off: recompute and overwrite files), or 1 (keep existing files). Defaults to 0.\n";
        exit(1);
    }
    std::string subjects_file = std::string(argv[1]);
    std::string subjects_dir = ".";
    std::string surface_name = "pial";
    bool do_circle_stats = false;
    bool keep_existing_files = false;
    bool circle_stats_do_meandists = false;
    if(argc >= 3) {
        subjects_dir = std::string(argv[2]);
    }
    if(argc >= 4) {
        surface_name = std::string(argv[3]);
    }
    if(argc >= 5) {
        if (std::string(argv[4]) == "1") {
            do_circle_stats = true;
        } else if (std::string(argv[4]) == "2") {
            do_circle_stats = true;
            circle_stats_do_meandists = true;
        } else if((std::string(argv[4]) == "0")) {
            // no-op, default.
        }  else {
            std::cerr << "Invalid value for parameter 'do_circle_stats'.\n";
            exit(1);
        }
    }
    if(argc == 6) {
        if (std::string(argv[5]) == "0") {
            // no-op, default.
        } else if (std::string(argv[5]) == "1") {
            keep_existing_files = true;
        } else {
            std::cerr << "Invalid value for parameter 'keep_existing'.\n";
            exit(1);
        }
    }


    const std::vector<std::string> subjects = fs::read_subjectsfile(subjects_file);
    std::cout << "Found " << subjects.size() << " subjects listed in subjects file '" << subjects_file << "'.\n";
    std::cout << "Using subject directory '" << subjects_dir << "' and surface '" << surface_name << "'.\n";
    const float circ_scale = 5.0; // The fraction of the total surface that the circles for the geodesic circle stats should have (in percent).
    std::cout << (do_circle_stats? "Computing" : "Not computing")  << " geodesic circle stats" << (do_circle_stats? " with scale " + std::to_string(circ_scale) : "") << ".\n";
    std::cout << (keep_existing_files? "Keeping" : "Not keeping (recomputing data for)")  << " existing output files.\n";
    if(do_circle_stats) {
        std::cout << (circle_stats_do_meandists? "Also computing" : "Not computing")  << " geodesic mean distances while computing circle stats.\n";
    }
    
    const std::vector<std::string> hemis = {"lh", "rh"};
    std::string surf_file, hemi, subject;
    fs::Mesh surface;

    for (size_t i=0; i<subjects.size(); i++) {
        subject = subjects[i];
        std::cout << " * Handling subject '" << subject << "', # " << (i+1) << " of " << subjects.size() << ".\n";
        for (size_t hemi_idx=0; hemi_idx<hemis.size(); hemi_idx++) {
            std::chrono::time_point<std::chrono::steady_clock> start_at = std::chrono::steady_clock::now();
            hemi = hemis[hemi_idx];
            
            // Load FreeSurfer mesh from file.
            surf_file = fs::util::fullpath({subjects_dir, subject, "surf", hemi + "." + surface_name});
            fs::read_mesh(&surface, surf_file);

            std::cout << "   - Handling hemi " << hemi << " for surface '" << surface_name << "' with " << surface.num_vertices() << " vertices and " << surface.num_faces() << " faces.\n";

            // Create a VCGLIB mesh from the libfs Mesh.            
            MyMesh m;
            vcgmesh_from_fs_surface(&m, surface);

            // Compute the geodesic mean distances and write result file.
            if(do_circle_stats) {
                const std::string rad_filename = fs::util::fullpath({subjects_dir, subject, "surf", hemi + ".geocirc_radius_vcglib_" + surface_name + ".curv"});
                const std::string per_filename = fs::util::fullpath({subjects_dir, subject, "surf", hemi + ".geocirc_perimeter_vcglib_" + surface_name + ".curv"});
                const std::string mgd_filename = fs::util::fullpath({subjects_dir, subject, "surf", hemi + ".mean_geodist_vcglib_" + surface_name + ".curv"});

                if(keep_existing_files) {
                    if(circle_stats_do_meandists) {
                        if(file_exists(rad_filename) && file_exists(per_filename) && file_exists(mgd_filename)) {
                            std::cout << "     o Skipping computation for hemi " << hemi << ", output files exist.\n";
                            continue;                            
                        }
                    } else {
                        if(file_exists(rad_filename) && file_exists(per_filename)) {
                            std::cout << "     o Skipping computation for hemi " << hemi << ", output files exist.\n";
                            continue;
                        }
                    }
                }

                std::vector<int32_t> qv_cs; // The query vertices (empty vector means to use all of the mesh).
                std::vector<std::vector<float>> circle_stats = geodesic_circles(m, qv_cs, circ_scale, circle_stats_do_meandists);
                const std::vector<float> radii = circle_stats[0];
                const std::vector<float> perimeters = circle_stats[1];                
                fs::write_curv(rad_filename, radii);
                std::cout << "     o Geodesic circle radius results for hemi " << hemi << " written to file '" << rad_filename << "'.\n";
                fs::write_curv(per_filename, perimeters);
                std::cout << "     o Geodesic circle perimeter results for hemi " << hemi << " written to file '" << per_filename << "'.\n";
                if(circle_stats_do_meandists) {
                    const std::vector<float> mean_geodists_circ = circle_stats[2];                    
                    fs::write_curv(mgd_filename, mean_geodists_circ);
                    std::cout << "     o Geodesic mean distance results for hemi " << hemi << " written to file '" << mgd_filename << "'.\n";
                }
            } else {
                const std::string mean_geodist_outfile = fs::util::fullpath({subjects_dir, subject, "surf", hemi + ".mean_geodist_vcglib_" + surface_name + ".curv"});
                if(keep_existing_files) {
                    if(file_exists(mean_geodist_outfile)) {
                        std::cout << "     o Skipping computation for hemi " << hemi << ", output file exists.\n";
                        continue;
                    }
                }                
                const std::vector<float> mean_dists = mean_geodist_p(m);                
                fs::write_curv(mean_geodist_outfile, mean_dists);
                std::cout << "     o Geodesic mean distance results for hemi " << hemi << " written to file '" << mean_geodist_outfile << "'.\n";
            }
            const std::chrono::time_point<std::chrono::steady_clock> end_at = std::chrono::steady_clock::now();
            const double duration_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end_at - start_at).count() / 1000.0;
            std::cout << "     o Computation for hemi " << hemi << " done after " << duration_seconds << " seconds.\n";
        }
    }

}
