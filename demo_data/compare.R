#!/usr/bin/env Rscript
#
# This script compares the output of the circle stats (perimeter and radius) of
# an R implementation of the circle stats ('R_output), the Matlab implementation
# by Gabriel Peyre ('ml_output'), and the C++ version from this repo ('cpp_vcg_output').
#
# This is for dev use, ignore it. I should turn this into a unit test.
# This requires R with the 'freesurferformats' and 'fsbrain' packages installed.

library("fsbrain");
library("freesurferformats");

# MAKE SURE that you have run the cpp version from this repo for fsaverage3 lh before starting this.

sjd = fsbrain::fsaverage.path(T);   # The SUBJECTS_DIR containing the data.
sj = "fsaverage3";                  # The subject to use.
do_vis = FALSE;                     # Whether to visualize the results (opens 3D interactive brain plot in new window).
datadir = "~/develop/cpp_geodesics/demo_data/";   # Path to the demo data from the repository on your machine.

if(! file.exists(file.path(datadir, "R_output/lh.fsaverage3_radius_s5.curv"))) {
    stop(sprintf("Incorrect 'datadir' setting '%s' in script, please adapt to your system.\n", datadir));
}

# Circle stats: radius -- Compare results for fsaverage3 lh
r_rad = freesurferformats::read.fs.curv(file.path(datadir, "R_output/lh.fsaverage3_radius_s5.curv"));
ml_rad = freesurferformats::read.fs.curv(file.path(datadir, "ml_output/lh.fsaverage3_radius_s5.curv"));
cpp_rad = freesurferformats::read.fs.curv(file.path(datadir, "cpp_vcg_output/lh.fsaverage3_radius_s5.curv"));

cat(sprintf("[RADIUS] Range for R is %s, for ML is %s, for C++ is %s.\n", paste(range(r_rad), collapse=" "), paste(range(ml_rad), collapse=" "), paste(range(cpp_rad), collapse=" ")));
cat(sprintf("[RADIUS] Correlaton with ML results is %f for R version, %f for C++version.\n", cor(r_rad, ml_rad), cor(cpp_rad, ml_rad)));
if(do_vis) {
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = r_rad);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = ml_rad);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = cpp_rad);
}

# Circle stats: perimeter -- Compare results for fsaverage3 lh
r_per = freesurferformats::read.fs.curv(file.path(datadir, "R_output/lh.fsaverage3_perimeter_s5.curv"));
ml_per = freesurferformats::read.fs.curv(file.path(datadir, "ml_output/lh.fsaverage3_perimeter_s5.curv"));
cpp_per = freesurferformats::read.fs.curv(file.path(datadir, "cpp_vcg_output/lh.fsaverage3_perimeter_s5.curv"));

cat(sprintf("[PERIMETER] Range for R is %s, for ML is %s, for C++ is %s.\n", paste(range(r_per), collapse=" "), paste(range(ml_per), collapse=" "), paste(range(cpp_per), collapse=" ")))
cat(sprintf("[PERIMETER] Correlaton with ML results is %f for R version, %f for C++version.\n", cor(r_per, ml_per), cor(cpp_per, ml_per)));
if(do_vis) {
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = r_per);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = ml_per);
    fsbrain::vis.data.on.subject(sjd, sj, morph_data_lh = cpp_per);
}

