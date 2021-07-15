#pragma once

#include "typedef_vcg.h"
#include <wrap/io_trimesh/export_ply.h>
#include <wrap/ply/plylib.h>
#include <vcg/complex/algorithms/pointcloud_normal.h>
#include <wrap/io_trimesh/export_vrml.h>
#include <wrap/io_trimesh/import.h>


void export_mesh_ply(MyMesh& m, const std::string& outfile) {
    std::cout << "Exporting mesh in PLY format to file '" << outfile << "'.\n";
    int mask = 0;   // The mask defines which mesh features to write (e.g., vertex colors, normals, ...)
    bool use_binary_format_version = false;
    vcg::tri::io::ExporterPLY<MyMesh>::Save(m, outfile.c_str(), mask, use_binary_format_version);
}
