#!/bin/bash
# Pre-compute mesh geodesic distances for FreeSurfer meshes and save them in JSON format.

subjects_dir="$FREESURFER_HOME/subjects"
apptag="[GEOD_DIST]"

for subject in fsaverage6 fsaverage; do
    if [ ! -d "${subjects_dir}/${subject}" ]; then
      echo "$apptag ERROR: Cannot read directory '${subjects_dir}/${subject}', exiting."
      exit 1
    fi
    for hemi in lh rh; do
        for surface in white; do
            for distance in 55; do
                echo "$apptag === Handling subject ${subject} hemi ${hemi} at distance ${distance}... ==="
                mesh_file="${subjects_dir}/${subject}/surf/${hemi}.${surface}"
                if [ ! -f "${mesh_file}" ]; then
                    echo "$apptag ERROR: Cannot read mesh file '${mesh_file}', exiting."
                    exit 1
                fi
                output_file="${subject}_${hemi}_${surface}_meshdist_geod_${distance}"
                ./meshneigh_geod "${mesh_file}" ${distance} "${output_file}" true && echo "$apptag Geodesic results written to file '${output_file}'".
            done
        done
    done
done

echo "$apptag All done, exiting."


