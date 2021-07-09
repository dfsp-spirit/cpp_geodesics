
// Demo program that reads per-vertex data from a curv file.
// To compile this witg g++ 9.3:
// 
//    g++ -I../../include/ read_curv.cpp -o read_curv
//
// or with clang 10:
//
//    clang++ -I../../include/ read_curv.cpp -o read_curv
//

#include "libfs.h"

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

int main(int argc, char** argv) {
    if(argc > 1) {
        std::cout << "Usage: " << argv[0] << "\n";
    }
    std::string lh_surf_file = "demo_data/subjects_dir/fsaverage3/surf/lh.white";
    std::string rh_surf_file = "demo_data/subjects_dir/fsaverage3/surf/rh.white";
    std::string lh_label_file = "demo_data/subjects_dir/fsaverage3/label/lh.cortex.label";
    std::string rh_label_file = "demo_data/subjects_dir/fsaverage3/label/rh.cortex.label";

    fs::Mesh lh_white, rh_white; 
    fs::read_fssurface(&lh_white, lh_surf_file);
    fs::read_fssurface(&rh_white, rh_surf_file);

    fs::Label lh_cortex, rh_cortex;
    fs::read_label(&lh_cortex, lh_label_file);
    fs::read_label(&rh_cortex, rh_label_file);

    std::cout << "Read surfaces and labels.\n";

    exit(0);
}
