#pragma once

#include "typedef_vcg.h"
#include <wrap/io_trimesh/export_ply.h>
#include <wrap/ply/plylib.h>
#include <vcg/complex/algorithms/pointcloud_normal.h>
#include <wrap/io_trimesh/export_vrml.h>
#include <wrap/io_trimesh/import.h>


double mesh_area_total(MyMesh& m) {
    FaceIterator face;
    double area = 0.0;
    for(face=m.face.begin(); face != m.face.end(); face++) {
      if(!(*face).IsD()) {
	    area += DoubleArea(*face);
      }
    }
    return(area/2.0);
}


std::vector<double> mesh_area_per_face(MyMesh& m) {
    FaceIterator face;
    std::vector<double> faceareas;    
    faceareas.resize(m.fn);
    int faceind = 0;
    for(face=m.face.begin(); face != m.face.end(); face++) {
      if(!(*face).IsD()) {
        faceareas[faceind] = DoubleArea(*face);
	    faceind++;
      }
    }
    return(faceareas);
}