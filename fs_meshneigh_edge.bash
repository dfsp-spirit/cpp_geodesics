#!/bin/bash
# Pre-compute mesh edge distances for FreeSurfer meshes and save them in JSON format.

### Settings

subjects_dir="$SUBJECTS_DIR"
apptag="[EDGE_DIST_BASH]"
subjects_list="fsaverage fsaverage6"
use_subjects_file="true"  # Set to "false" or "true".
subjects_file="${subjects_dir}/subjects.txt"
pvd_descriptor="pial_lgi" # Set to something like "thickness", "volume", "area", or the empty string "" for none.

### End of Settings

if [ "${use_subjects_file}" = "true" ]; then
    if [ ! -f "${subjects_file}"  ]; then
        echo "$apptag ERROR: Cannot read subjects file '${subjects_file}'. Exiting."
        exit 1
    else
        subjects_list=$(cat $subjects_file | tr '\n' ' ')
        log_tag_from=" from subjects file '${subjects_file}'"
    fi
else
    log_tag_from=" from hardcoded, internal subjects list"
fi

num_subjects=$(echo ${subjects_list} | wc -w)


# Positional trailing command line arguments for meshneigh_edge
include_self="true"
json="false"
csv="true"
vv="false"
with_neigh="true"
extra_args="${include_self} ${json} ${csv} ${vv} ${with_neigh}"

echo "$apptag INFO: Using meshneigh_edge extra_args '$extra_args'."
echo "$apptag INFO: Handling ${num_subjects} subjects${log_tag_from}."




num_handled=0
for subject in ${subjects_list}; do
    num_handled=$((num_handled+1))
    if [ ! -d "${subjects_dir}/${subject}" ]; then
      echo "$apptag ERROR: Cannot read directory '${subjects_dir}/${subject}', exiting."
      exit 1
    fi
    for hemi in lh rh; do
        for surface in pial; do
            for distance in 5; do
                echo "$apptag === Handling subject #${num_handled} of ${num_subjects} named '${subject}': hemi ${hemi} surface ${surface} at distance ${distance}... ==="
                mesh_file="${subjects_dir}/${subject}/surf/${hemi}.${surface}"
                if [ ! -f "${mesh_file}" ]; then
                    echo "$apptag ERROR: Cannot read mesh file '${mesh_file}', exiting."
                    exit 1
                fi
                output_file="${subject}_${hemi}_${surface}_meshdist_edge_${distance}"

                pvd_descriptor_file=""
                if [ -n "${pvd_descriptor}" ]; then
                    pvd_descriptor_file="${subjects_dir}/${subject}/surf/${hemi}.${pvd_descriptor}"
                fi

                ./meshneigh_edge "${mesh_file}" ${distance} "${output_file}" ${extra_args} ${pvd_descriptor_file} && echo "$apptag Edge results for hemi $hemi surface $surface distance $distance written to file '${output_file}'".
            done
        done
    done
done

echo "$apptag INFO: All ${num_subjects} subjects${log_tag_from} done, exiting."


