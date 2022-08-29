#pragma once

#include "typedef_vcg.h"


/// @brief Compute vertex normals of VDGLIB mesh.
/// @param m The VCGLIB mesh
/// @param face_angle_weighted the vertex normals type, angle_weighted (true) or area weighted average of surrounding face normals (false).
std::vector<std::vector<float>> mesh_vnormals(MyMesh& m, const bool face_angle_weighted=false) {

    if(face_angle_weighted) {
        tri::UpdateNormal<MyMesh>::PerVertexAngleWeighted(m);
    } else {
        tri::UpdateNormal<MyMesh>::PerVertex(m);
    }

    FaceIterator face;
    double area = 0.0;
    for(face=m.face.begin(); face != m.face.end(); face++) {
      if(!(*face).IsD()) {
	      area += DoubleArea(*face);
      }
    }

    VertexIterator vi;
    vi = m.vert.begin();

    std::vector<std::vector<float>> vnormals = std::vector<std::vector<float>>(m.vn, std::vector<float>(3, 0.0));
    for (int i=0;  i < m.vn; i++) {
      if( ! vi->IsD() )	{
        vnormals[i][0] = (*vi).N()[0];
        vnormals[i][1] = (*vi).N()[1];
        vnormals[i][2] = (*vi).N()[2];
      }
      ++vi;
    }

    return vnormals;
}