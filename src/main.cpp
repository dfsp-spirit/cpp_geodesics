
// The main for our code.

#include "libfs.h"

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

    std::cout << "Reading surfaces and labels...\n";

    std::string lh_surf_file = "demo_data/subjects_dir/fsaverage3/surf/lh.white";
    std::string rh_surf_file = "demo_data/subjects_dir/fsaverage3/surf/rh.white";
    std::string lh_label_file = "demo_data/subjects_dir/fsaverage3/label/lh.cortex.label";
    std::string rh_label_file = "demo_data/subjects_dir/fsaverage3/label/rh.cortex.label";

    fs::Mesh lh_white, rh_white; 
    fs::read_surf(&lh_white, lh_surf_file);
    fs::read_surf(&rh_white, rh_surf_file);

    fs::Label lh_cortex, rh_cortex;
    fs::read_label(&lh_cortex, lh_label_file);
    fs::read_label(&rh_cortex, rh_label_file);

    std::cout << " * Done reading surfaces and labels.\n";

    exit(0);
}
