#!/usr/bin/env Rscript

library("fsbrain");
library("freesurferformats");

# MAKE SURE that you have run the cpp version for fsaverage3 lh before starting this.

sjd = fsbrain::fsaverage.path(T);
sj = "fsaverage3";
do_vis = F;

# Load results for fsaverage3 lh
r_rad = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/demo_data/R_output/lh.fsaverage3_radius_s5.curv");
ml_rad = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/demo_data/ml_output/lh.fsaverage3_radius_s5.curv");
cpp_rad = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/lh.fsaverage3_radius_s5.curv");

cat(sprintf("[RADIUS] Range for R is %s, for ML is %s, for C++ is %s.\n", paste(range(r_rad), collapse=" "), paste(range(ml_rad), collapse=" "), paste(range(cpp_rad), collapse=" ")));
cat(sprintf("[RADIUS] Correlaton with ML results is %f for R version, %f for C++version.\n", cor(r_rad, ml_rad), cor(cpp_rad, ml_rad)));
if(do_vis) {
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = r_rad);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = ml_rad);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = cpp_rad);
}

# Load results for fsaverage3 lh
r_per = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/demo_data/R_output/lh.fsaverage3_perimeter_s5.curv");
ml_per = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/demo_data/ml_output/lh.fsaverage3_perimeter_s5.curv");
cpp_per = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/lh.fsaverage3_perimeter_s5.curv");

cat(sprintf("[PERIMETER] Range for R is %s, for ML is %s, for C++ is %s.\n", paste(range(r_per), collapse=" "), paste(range(ml_per), collapse=" "), paste(range(cpp_per), collapse=" ")))
cat(sprintf("[PERIMETER] Correlaton with ML results is %f for R version, %f for C++version.\n", cor(r_per, ml_per), cor(cpp_per, ml_per)));
if(do_vis) {
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = r_per);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = ml_per);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = cpp_per);
}

