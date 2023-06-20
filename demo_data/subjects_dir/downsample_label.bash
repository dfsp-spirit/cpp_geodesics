#!/bin/bash
# downsample a label, e.g., downsample lh.cortex.label (ico7) to lh.cortex6.label (ico6) for a subject.
# The downsampled label can be used to find the cortical vertices (as opposed to the medial wall) for a subject on its ico6 surface.
# Usage: bash downsample_label.bash

SUBJECT="subject1"
LABEL="cortex"
ICO_ORDER=6

mri_label2label --srclabel ./${SUBJECT}/label/lh.cortex.label --srcsubject "${SUBJECT}" --trglabel ./${SUBJECT}/label/lh.cortex${ICO_ORDER}.label --trgsubject ico --regmethod surface --hemi lh --trgicoorder ${ICO_ORDER}
mri_label2label --srclabel ./${SUBJECT}/label/rh.cortex.label --srcsubject "${SUBJECT}" --trglabel ./${SUBJECT}/label/rh.cortex${ICO_ORDER}.label --trgsubject ico --regmethod surface --hemi rh --trgicoorder ${ICO_ORDER}