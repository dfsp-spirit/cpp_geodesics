#!/bin/bash
#
# Runs a quick test of the geodcircles program on an ICO4 mesh, and exports the resulting overlays to colored meshes in PLY format.


../../geodcircles subjects.txt . pialsurface4 2 1 2
../../export_brainmesh subject1/surf/lh.pialsurface4 subject1/surf/lh.geocirc_perimeter_vcglib_pialsurface4_circscale2.curv lh.pial4_perimeter.ply
../../export_brainmesh subject1/surf/lh.pialsurface4 subject1/surf/lh.geocirc_radius_vcglib_pialsurface4_circscale2.curv lh.pial4_radius.ply
../../export_brainmesh subject1/surf/lh.pialsurface4 subject1/surf/lh.mean_geodist_vcglib_pialsurface4.curv lh.pial4_meandist.ply


