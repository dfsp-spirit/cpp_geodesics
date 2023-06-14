
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
        std::cout << "  <do_circle_stat>: flag whether to compute geodesic circle stats as well, must be 0 (off), 1 (on) or 2 (on with mean dists). Defaults to 2.\n";
        std::cout << "  <keep_existing> : flag whether to keep existing output files, must be 0 (off: recompute and overwrite files), or 1 (keep existing files). Defaults to 1.\n";
        exit(1);
    }
    std::string subjects_file = std::string(argv[1]);
    std::string subjects_dir = ".";
    std::string surface_name = "pial";
    bool do_circle_stats = true;
    bool keep_existing_files = true;
    bool circle_stats_do_meandists = true;
    if(argc >= 3) {
        subjects_dir = std::string(argv[2]);
    }
    if(argc >= 4) {
        surface_name = std::string(argv[3]);
    }
    if(argc >= 5) {
        if (std::string(argv[4]) == "1") {
            do_circle_stats = true;
            circle_stats_do_meandists = false;
        } else if (std::string(argv[4]) == "2") {
            do_circle_stats = true;
            circle_stats_do_meandists = true;
        } else if((std::string(argv[4]) == "0")) {
            do_circle_stats = false;
            circle_stats_do_meandists = false;
        }  else {
            std::cerr << "Invalid value for parameter 'do_circle_stats' Must be '0', '1' or '2'.\n";
            exit(1);
        }
    }
    if(argc == 6) { // whether to keep existing files / skip computation for those that are already done.
        if (std::string(argv[5]) == "0") {
            keep_existing_files = false;
        } else if (std::string(argv[5]) == "1") {
            keep_existing_files = true;
        } else {
            std::cerr << "Invalid value for parameter 'keep_existing'. Must be '0' or '1'.\n";
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

    const std::chrono::time_point<std::chrono::steady_clock> all_subjects_start_at = std::chrono::steady_clock::now();
    std::vector<std::string> failed_subjects; // To keep track of skipped subjects, e.g., because their required files could not be loaded. This does NOT include subjects which were skipped because the data was already there.
    unsigned int num_skipped_hemis_for_far = 0; // This counts subject hemispheres for which no computation took place: the sum of failed subject hemis (e.g., due to missing surface files) and (if 'keep_existing_files' is true) subject hemis for which the output files already existed.

    for (size_t i=0; i<subjects.size(); i++) {
        subject = subjects[i];
        std::cout << " * Handling subject '" << subject << "', # " << (i+1) << " of " << subjects.size() << ".\n";
        std::chrono::time_point<std::chrono::steady_clock> subject_start_at = std::chrono::steady_clock::now();
        for (size_t hemi_idx=0; hemi_idx<hemis.size(); hemi_idx++) {
            std::chrono::time_point<std::chrono::steady_clock> subject_hemi_start_at = std::chrono::steady_clock::now();
            hemi = hemis[hemi_idx];

            // Load FreeSurfer mesh from file.
            surf_file = fs::util::fullpath({subjects_dir, subject, "surf", hemi + "." + surface_name});
            try {
                fs::read_mesh(&surface, surf_file);
            } catch(const std::exception& e) {
                std::cerr << "   - Failed to load surface '" << surf_file << "' for subject " << subject << ", skipping hemi. Details: " << e.what();
                failed_subjects.push_back(subject); // This may result in subjects ending up twice in the list, if both hemis fail. That is fine with us for now, and handled at the end when reporting.
                num_skipped_hemis_for_far++;
                continue;
            }

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
                            num_skipped_hemis_for_far++;
                            continue;
                        }
                    } else {
                        if(file_exists(rad_filename) && file_exists(per_filename)) {
                            std::cout << "     o Skipping computation for hemi " << hemi << ", output files exist.\n";
                            num_skipped_hemis_for_far++;
                            continue;
                        }
                    }
                }

                // TODO: we could support ignoring the medial wall by reading ?h.cortex.label and
                //  passing it to 'geodesic_circles()' below. That function needs to pass it on further until
                //  we call VCGLIB's PerVertexDijkstraCompute() function. We could select vertices in the VCG mesh (using vertex.setS(), see http://vcglib.net/flags.html)
                //  prior to that call, and use the parameter 'avoid_selected' of PerVertexDijkstraCompute to ignore the selected medial wall verts. 
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
                        num_skipped_hemis_for_far++;
                        continue;
                    }
                }
                const std::vector<float> mean_dists = mean_geodist_p(m);
                fs::write_curv(mean_geodist_outfile, mean_dists);
                std::cout << "     o Geodesic mean distance results for hemi " << hemi << " written to file '" << mean_geodist_outfile << "'.\n";
            }
            const std::chrono::time_point<std::chrono::steady_clock> subject_hemi_end_at = std::chrono::steady_clock::now();
            const double hemi_duration_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(subject_hemi_end_at - subject_hemi_start_at).count() / 1000.0;
            std::cout << "     o Computation for hemi " << hemi << " done after " << hemi_duration_seconds << " seconds (" << secduration(hemi_duration_seconds) << ").\n";
        }
        const std::chrono::time_point<std::chrono::steady_clock> subject_end_at = std::chrono::steady_clock::now();
        const double subject_duration_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(subject_end_at - subject_start_at).count() / 1000.0;
        const double subjects_so_far_duration_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(subject_end_at - all_subjects_start_at).count() / 1000.0;
        std::cout << "   - Subject " << subject << " took " << secduration(subject_duration_seconds) << ".\n";
        const int num_hemis_computed = ((i+1) * 2) - num_skipped_hemis_for_far; // These are all hemispheres for which computational effort was needed (they were not skipped, for whatever reasons).
        if(i < (subjects.size() - 1) && num_hemis_computed > 0) {   // We do not give a time left estimate if nothing was really done yet or if we are finished.
            const double num_subjects_computed = num_hemis_computed / 2.0;
            const double estimated_time_left = subjects_so_far_duration_seconds / num_subjects_computed * (subjects.size() - num_subjects_computed);
            std::cout << "   - Duration since start " << secduration(subjects_so_far_duration_seconds) << " for " << (i+1) << " subjects. Estimated time left " << secduration(estimated_time_left) << " for " << (subjects.size() - (i+1)) << " subjects.\n";
        }

    }

    // Report on failed subjects (e.g., failed due to missing files).
    if(failed_subjects.size() > 0) {
        // We need to make failed_subjects unique, as it may contain a subject twice if both of its hemispheres failed.
        std::sort( failed_subjects.begin(), failed_subjects.end() );
        failed_subjects.erase( unique( failed_subjects.begin(), failed_subjects.end() ), failed_subjects.end() );
        std::cout << "Computation failed for " << failed_subjects.size() << " of the " << subjects.size() << " subjects:\n";
        for (const auto& subj: failed_subjects) {
            std::cout << subj << ' ';
        }
        std::cout << '\n';
    } else {
        std::cout << "Computation succeeded for all " << subjects.size() << " subjects.\n";
    }

}
