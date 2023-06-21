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

# Parameters for geodcircles:
do_circle_stats=2  # with mean dists
keep_existing=0    # off
circ_scale=2       # 2 percent of total area

if [ ! -f subjects.txt ]; then
    echo "${APPTAG} ERROR: File subjects.txt not found. Please run this script from the directory it is stored in."
    exit 1
fi


if [ -n "$1" ]; then
    surface=$1
    if [ "${surface}" == "pial" -o "${surface}" == "pialsurface4" -o "${surface}" == "pialsurface5" -o "${surface}" == "pialsurface6"]; then
        echo "${APPTAG} Using surface ${surface} from command line."
    else
        echo "${APPTAG} ERROR: Parameter 'surface' must be 'pialsurface4', 'pialsurface5', 'pialsurface6', or 'pial'."
        echo "${APPTAG} USAGE: $0 [surface]"
        exit 1
    fi
else
    surface="pialsurface4"
    echo "${APPTAG} Using surface ${surface}."
fi

if [ "${surface}" == "pialsurface4" ]; then
    ico_order=4
    cortex_label_file="cortex${ico_order}.label"
elif [ "${surface}" == "pialsurface5" ]; then
    ico_order=5
    cortex_label_file="cortex${ico_order}.label"
elif [ "${surface}" == "pialsurface6" ]; then
    ico_order=6
    cortex_label_file="cortex${ico_order}.label"
elif [ "${surface}" == "pial" ]; then
    ico_order=7
    cortex_label_file="cortex.label"
fi

echo "${APPTAG} Running geodcircles on ${surface} surface with ico_order=${ico_order}..."
echo "${APPTAG} Using cortex label file ${cortex_label_file}."

../../geodcircles subjects.txt . ${surface} $do_circle_stats $keep_existing $circ_scale ${cortex_label_file}
../../export_brainmesh subject1/surf/lh.${surface} subject1/surf/lh.geocirc_perimeter_vcglib_${surface}_circscale2.curv lh.${surface}_perimeter.ply
../../export_brainmesh subject1/surf/lh.${surface} subject1/surf/lh.geocirc_radius_vcglib_${surface}_circscale2.curv lh.${surface}_radius.ply
../../export_brainmesh subject1/surf/lh.${surface} subject1/surf/lh.mean_geodist_vcglib_${surface}.curv lh.${surface}_meandist.ply

echo "$APPTAG Done. You can view the results in MeshLab or other tools, e.g.: meshlab lh.${surface}_meandist.ply"

