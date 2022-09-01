#!/bin/bash
# Pre-compute mesh edge distances for FreeSurfer meshes and save them in JSON format.

subjects_dir="$FREESURFER_HOME/subjects"
apptag="[EDGE_DIST]"

# Positional trailing command line arguments for meshneigh_edge
include_self="true"
json="false"
csv="true"
vv="false"
with_neigh="true"
extra_args="${include_self} ${json} ${csv} ${vv} ${with_neigh}"

echo "$apptag Info: Using meshneigh_edge extra_args '$extra_args'."

for subject in fsaverage fsaverage6; do
    if [ ! -d "${subjects_dir}/${subject}" ]; then
      echo "$apptag ERROR: Cannot read directory '${subjects_dir}/${subject}', exiting."
      exit 1
    fi
    for hemi in lh rh; do
        for surface in white; do
            for distance in 1 2 3 4 5; do
                echo "$apptag === Handling subject ${subject} hemi ${hemi} surface ${surface} at distance ${distance}... ==="
                mesh_file="${subjects_dir}/${subject}/surf/${hemi}.${surface}"
                if [ ! -f "${mesh_file}" ]; then
                    echo "$apptag ERROR: Cannot read mesh file '${mesh_file}', exiting."
                    exit 1
                fi
                output_file="${subject}_${hemi}_${surface}_meshdist_edge_${distance}"
                ./meshneigh_edge "${mesh_file}" ${distance} "${output_file}" ${extra_args} && echo "$apptag Edge results for hemi $hemi surface $surface distance $distance written to file '${output_file}'".
            done
        done
    done
done

echo "$apptag All done, exiting."


