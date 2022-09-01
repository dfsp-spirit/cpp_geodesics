#!/bin/bash
# Pre-compute mesh geodesic distances for FreeSurfer meshes and save them to files.
#
# IMPORTANT: The distance you chose below must fit to the mesh! Note that for FreeSurfer meshes,
#            the suitable distance differs based on the mesh ICO value (simplification factor of the
#            mesh), as meshes with fewer vertices (like fsaverage6 compared to the full fsaverage) will
#            have longer edges! Running geodesic neighborhood computation with a too low 'distance' value
#            means that the neighborhoods will be empty (or very small), because no (or only very few) vertices
#            will be found within the max query distance!

subjects_dir="$FREESURFER_HOME/subjects"
apptag="[GEOD_DIST]"

# Positional trailing command line arguments for meshneigh_geod
include_self="true"
json="false"
csv="true"
vv="false"
with_neigh="true"
extra_args="${include_self} ${json} ${csv} ${vv} ${with_neigh}"

for subject in fsaverage6 fsaverage; do
    if [ ! -d "${subjects_dir}/${subject}" ]; then
      echo "$apptag ERROR: Cannot read directory '${subjects_dir}/${subject}', exiting."
      exit 1
    fi
    for hemi in lh rh; do
        for surface in white; do
            for distance in 25; do
                echo "$apptag === Handling subject ${subject} hemi ${hemi} at distance ${distance}... ==="
                mesh_file="${subjects_dir}/${subject}/surf/${hemi}.${surface}"
                if [ ! -f "${mesh_file}" ]; then
                    echo "$apptag ERROR: Cannot read mesh file '${mesh_file}', exiting."
                    exit 1
                fi
                output_file="${subject}_${hemi}_${surface}_meshdist_geod_${distance}"
                ./meshneigh_geod "${mesh_file}" ${distance} "${output_file}" ${extra_args} && echo "$apptag Geodesic results written to file '${output_file}'".
            done
        done
    done
done

echo "$apptag All done, exiting."


