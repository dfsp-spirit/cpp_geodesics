#!/bin/bash
# Pre-compute mesh distances for FreeSurfder meshes and save them in JSON format.

subjects_dir="$FREESURFER_HOME/subjects"

for subject in fsaverage fsaverage6; do
    if [ ! -d "${subjects_dir}/${subject}" ]; then
      echo "ERROR: Cannot read directory '${subjects_dir}/${subject}', exiting."
      exit 1
    fi
    for hemi in lh rh; do    
        for distance in 1 2 3 4 5; do
            echo "Handling subject ${subject} hemi ${hemi} at distance ${distance}..."
            mesh_file="${subjects_dir}/${subject}/surf/${hemi}.white"
            if [ ! -f "${mesh_file}" ]; then
                echo "ERROR: Cannot read mesh file '${mesh_file}', exiting."
                exit 1
            fi
            ./meshneigh_edge "${mesh_file}" ${distance} "${subject}_meshdist_edge_${distance}.json" true
        done
    done
done

echo "All done, exiting."


