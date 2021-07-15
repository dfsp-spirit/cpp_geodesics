#pragma once

#include "typedef_vcg.h"
#include <wrap/io_trimesh/export_ply.h> // for ExporterPLY t save to PLY format
#include <wrap/ply/plylib.h> // for ExporterPLY t save#include <wrap/io_trimesh/import.h>
#include <vcg/complex/algorithms/pointcloud_normal.h>
#include <wrap/io_trimesh/export_vrml.h>
#include <wrap/io_trimesh/import.h>


void export_mesh_ply(MyMesh& m, const std::string& outfile) {
    std::cout << "Exporting mesh in PLY format to file '" << outfile << "'.\n";
    vcg::tri::io::ExporterPLY<MyMesh>::Save(m, outfile.c_str());
}
