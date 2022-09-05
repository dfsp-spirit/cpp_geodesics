#!/bin/bash
# Pre-compute mesh edge distances for FreeSurfer meshes and save them in JSON format.

### Settings

subjects_dir="$SUBJECTS_DIR"
apptag="[EDGE_DIST_BASH]"
subjects_list="fsaverage fsaverage6" # Ignored if 'use_subjects_file=true'.
use_subjects_file="true"  # Set to "false" or "true".
subjects_file="${subjects_dir}/subjects.txt"
pvd_descriptor="pial_lgi" # Set to something like "thickness", "volume", "area", or the empty string "" for none.
run_with_gnu_parallel="true"
gnu_parallel_num_parallel_jobs=$(nproc)

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


if [ "${run_with_gnu_parallel}" = "true" ]; then

    echo "$APPTAG Running ${gnu_parallel_num_parallel_jobs} jobs in in parallel with 'GNU Parallel'."
    date_tag=$(date '+%Y-%m-%d_%H-%M-%S')
    for hemi in lh rh; do
        for surface in pial; do
            for distance in 5; do
                echo ${subjects_list} | tr ' ' '\n' | parallel --jobs ${gnu_parallel_num_parallel_jobs} --workdir . --joblog LOGFILE_MESHNEIGH_EDGE__PARALLEL_${hemi}_${surface}_${distance}_${date_tag}.txt \
                ./meshneigh_edge "${subjects_dir}/"{}"/surf/${hemi}.${surface}" $distance {}"_${hemi}_${surface}_meshdist_edge_${distance}" ${extra_args} "${subjects_dir}/"{}"/surf/${hemi}.${pvd_descriptor}"
            done
        done
    done
else
    echo "$APPTAG Running sequentially."

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
fi

echo "$apptag INFO: All ${num_subjects} subjects${log_tag_from} done, exiting."


