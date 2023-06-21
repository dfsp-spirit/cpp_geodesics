#!/bin/bash
#
# Runs a quick test of the geodcircles program on an ICO4 mesh, and exports the resulting overlays to colored meshes in PLY format.
#
# Make sure you have compiled the geodcircles program and the export_brainmesh program.
# Run this from the directory it is stored in:
#
#  ./testrun_pial4.bash
#
# or:
#
#  ./testrun_pial4.bash pialsurface5

APPTAG="[TESTRUN]"

if [ -n "$1" ]; then
    surface=$1
    if [ "${surface}" == "pialsurface4" -o "${surface}" == "pialsurface5" -o "${surface}" == "pialsurface6"]; then
        echo "${APPTAG} Using surface ${surface} from command line."
    else
        echo "${APPTAG} ERROR: Parameter 'surface' must be 'pialsurface4', 'pialsurface5', or 'pialsurface6'."
        echo "${APPTAG} USAGE: $0 [surface]"
        exit 1
    fi
else
    surface="pialsurface4"
    echo "${APPTAG} Using surface ${surface}."
fi

../../geodcircles subjects.txt . ${surface} 2 1 2
../../export_brainmesh subject1/surf/lh.${surface} subject1/surf/lh.geocirc_perimeter_vcglib_${surface}_circscale2.curv lh.${surface}_perimeter.ply
../../export_brainmesh subject1/surf/lh.${surface} subject1/surf/lh.geocirc_radius_vcglib_${surface}_circscale2.curv lh.${surface}_radius.ply
../../export_brainmesh subject1/surf/lh.${surface} subject1/surf/lh.mean_geodist_vcglib_${surface}.curv lh.${surface}_meandist.ply

echo "$APPTAG Done. You can view the results in MeshLab or other tools, e.g.: meshlab lh.${surface}_meandist.ply"

