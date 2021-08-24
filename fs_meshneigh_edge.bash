#!/bin/bash
# Pre-compute mesh edge distances for FreeSurfer meshes and save them in JSON format.

subjects_dir="$FREESURFER_HOME/subjects"
apptag="[EDGE_DIST]"

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
                ./meshneigh_edge "${mesh_file}" ${distance} "${output_file}" true && echo "$apptag Edge results written to file '${output_file}'".
            done
        done
    done
done

echo "$apptag All done, exiting."


