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
        for distance in 1 2 3 4 5; do
            echo "$apptag === Handling subject ${subject} hemi ${hemi} at distance ${distance}... ==="
            mesh_file="${subjects_dir}/${subject}/surf/${hemi}.white"
            if [ ! -f "${mesh_file}" ]; then
                echo "$apptag ERROR: Cannot read mesh file '${mesh_file}', exiting."
                exit 1
            fi
            ./meshneigh_edge "${mesh_file}" ${distance} "${subject}_meshdist_edge_${distance}.json" true
        done
    done
done

echo "$apptag All done, exiting."


