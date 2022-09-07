#!/bin/bash
# Pre-compute mesh edge distances for FreeSurfer meshes and save them in JSON format.

### Settings

subjects_dir="$SUBJECTS_DIR"
apptag="[EDGE_DIST_BASH]"
#subjects_list="fsaverage fsaverage6" # Ignored if 'use_subjects_file=true'.
subjects_list="Caltech_0051459"
use_subjects_file="false"  # Set to "false" or "true".
subjects_file="${subjects_dir}/subjects.txt"
pvd_descriptor="pial_lgi" # Set to something like "thickness", "volume", "area", or the empty string "" for none.
run_with_gnu_parallel="true"
gnu_parallel_num_parallel_jobs=$(nproc)
output_dir="output" # Must exist. Set to '.' for current directory. Do not leave completely empty.

# Positional trailing command line arguments for meshneigh_edge
include_self="true"
json="false"
csv="true"
vv="false"
with_neigh="true"
extra_args="${include_self} ${json} ${csv} ${vv} ${with_neigh}"

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

if [ -z "${output_dir}" ]; then
    output_dir="."
    echo "$apptag WARNING: Adjusted empty 'output_dir' setting to '${output_dir}'."
fi

if [ ! -d "${output_dir}" ]; then
    echo "$apptag ERROR: Output directory '${output_dir}' does not exist or cannot be read."
    exit 1
fi

if [ $num_subjects -eq 1 ]; then
    if [ "${run_with_gnu_parallel}" = "true" ]; then
        run_with_gnu_parallel="false"
        echo "$apptag INFO: Disabling parallel mode for processing of ${num_subjects} subjects."
    fi
fi

echo "$apptag INFO: Using meshneigh_edge extra_args '$extra_args'."
echo "$apptag INFO: Handling ${num_subjects} subjects${log_tag_from}."


if [ "${run_with_gnu_parallel}" = "true" ]; then

    echo "$apptag Running ${gnu_parallel_num_parallel_jobs} jobs in in parallel with 'GNU Parallel'."
    for hemi in lh rh; do
        for surface in pial; do
            for distance in 5; do
                start_date_tag=$(date '+%Y-%m-%d_%H-%M-%S')
                echo "$apptag Running in parallel for hemi '${hemi}', surface ${surface} with distance '${distance}'."
                echo ${subjects_list} | tr ' ' '\n' | parallel --jobs ${gnu_parallel_num_parallel_jobs} --workdir . --joblog LOGFILE_MESHNEIGH_EDGE__PARALLEL_${hemi}_${surface}_${distance}_${start_date_tag}.txt ./meshneigh_edge "${subjects_dir}/"{}"/surf/${hemi}.${surface}" $distance "${output_dir}/"{}"_${hemi}_${surface}_meshdist_edge_${distance}" ${extra_args} "${subjects_dir}/"{}"/surf/${hemi}.${pvd_descriptor}" "${subjects_dir}/"{}"/label/${hemi}.cortex.label"
                end_date_tag=$(date '+%Y-%m-%d_%H-%M-%S')
                echo "Done running in parallel for hemi '${hemi}', surface ${surface} with distance '${distance}'. Startet at '${start_date_tag}', done at '${end_date_tag}'."
            done
        done
    done
else
    start_date_tag=$(date '+%Y-%m-%d_%H-%M-%S')
    echo "$apptag Running sequentially, starting at '${start_date_tag}'."

    num_handled=0
    for subject in ${subjects_list}; do
        num_handled=$((num_handled+1))
        if [ ! -d "${subjects_dir}/${subject}" ]; then
        echo "$apptag ERROR: Cannot read directory '${subjects_dir}/${subject}', exiting."
        exit 1
        fi
        for hemi in lh rh; do
            this_hemi_done_start_tag=$(date '+%Y-%m-%d_%H-%M-%S')
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

                    cortex_label_file="${subjects_dir}/${subject}/label/${hemi}.cortex.label"

                    ./meshneigh_edge "${mesh_file}" ${distance} "${output_dir}/${output_file}" ${extra_args} ${pvd_descriptor_file} ${cortex_label_file} && echo "$apptag Edge results for hemi $hemi surface $surface distance $distance written to file '${output_file}'".
                done
            done
            this_hemi_done_date_tag=$(date '+%Y-%m-%d_%H-%M-%S')
            echo "$apptag Data for hemi '${hemi}' done, started at '${this_hemi_done_start_tag}', done now at '${this_hemi_done_date_tag}'."
        done
    done
    end_date_tag=$(date '+%Y-%m-%d_%H-%M-%S')
    echo "$apptag Done running sequentially, started at '${start_date_tag}', done at '${end_date_tag}'."
fi

echo "$apptag INFO: All ${num_subjects} subjects${log_tag_from} done at '${end_date_tag}', exiting."


