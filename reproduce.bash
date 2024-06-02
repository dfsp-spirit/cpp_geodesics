#!/bin/bash
#
# reproduce.bash
#
# This script runs the entire pipeline for computing geodesics for a subject, using the down-scaled (fsaverage6-sized) meshes.
# This includes:
#  * down-sampling the native subjects mesh from full size to fsaverage6 size
#  * down-sampling the native cortex labels from full size to fsaverage6 size
#  * computing the descriptors
#
# This script assumes that you have already run the full FreeSurfer `recon-all` pipeline for the subject.
# It also assumes that FreeSurfer is installed and setup correctly for the bash shell. If not, you will
# get errors about 'command not found' for various FreeSurfer tools below, and this script will fail.
#
# It also assumes that you have already compiled the `geodcircles` application.
#


cd demo_data/subjects_dir/
export SUBJECTS_DIR=$(pwd)

SUBJECT="subject1"

# Downsample native space meshes from full resolution to fsaverage6 resolution
mri_surf2surf --hemi lh --srcsubject ${SUBJECT} --sval-xyz pial --trgsubject fsaverage6 --trgicoorder 6 --tval-xyz ${SUBJECT}/mri/brain.mgz --tval ${SUBJECT}/surf/lh.pialsurface6
mri_surf2surf --hemi rh --srcsubject ${SUBJECT} --sval-xyz pial --trgsubject fsaverage6 --trgicoorder 6 --tval-xyz ${SUBJECT}/mri/brain.mgz --tval ${SUBJECT}/surf/rh.pialsurface6


# Downsample native space labels
LABEL="cortex"
ICO_ORDER=6
lh_output_label="./${SUBJECT}/label/lh.${LABEL}${ICO_ORDER}.label"
rh_output_label="./${SUBJECT}/label/rh.${LABEL}${ICO_ORDER}.label"
mri_label2label --srclabel ./${SUBJECT}/label/lh.${LABEL}.label --srcsubject "${SUBJECT}" --trglabel "${lh_output_label}" --trgsubject ico --regmethod surface --hemi lh --trgicoorder ${ICO_ORDER}
mri_label2label --srclabel ./${SUBJECT}/label/rh.${LABEL}.label --srcsubject "${SUBJECT}" --trglabel "${rh_output_label}" --trgsubject ico --regmethod surface --hemi rh --trgicoorder ${ICO_ORDER}

# Compute descriptors
./geodcircles demo_data/subjects_dir/subjects.txt demo_data/subjects_dir/

