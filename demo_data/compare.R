#!/usr/bin/env Rscript

library("fsbrain");
library("freesurferformats");

# MAKE SURE that you have run the cpp version for fsaverage3 lh before starting this.

sjd = fsbrain::fsaverage.path(T);
sj = "fsaverage3";
do_vis = TRUE;

# Load results for fsaverage3 lh
r_rad = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/demo_data/R_output/lh.fsaverage3_radius_s5.curv");
ml_rad = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/demo_data/ml_output/lh.fsaverage3_radius_s5.curv");
cpp_rad = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/geocircles_radius.curv");

cat(sprintf("[RADIUS] Range for R is %s, for ML is %s, for C++ is %s.\n", paste(range(r_rad), collapse=" "), paste(range(ml_rad), collapse=" "), paste(range(cpp_rad), collapse=" ")))
if(do_vis) {
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = r_rad);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = ml_rad);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = cpp_rad);
}

# Load results for fsaverage3 lh
r_per = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/demo_data/R_output/lh.fsaverage3_perimeter_s5.curv");
ml_per = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/demo_data/ml_output/lh.fsaverage3_perimeter_s5.curv");
cpp_per = freesurferformats::read.fs.curv("~/develop/cpp_geodesics/geocircles_perimeter.curv");

cat(sprintf("[PERIMETER] Range for R is %s, for ML is %s, for C++ is %s.\n", paste(range(r_per), collapse=" "), paste(range(ml_per), collapse=" "), paste(range(cpp_per), collapse=" ")))

if(do_vis) {
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = r_per);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = ml_per);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = cpp_per);
}

