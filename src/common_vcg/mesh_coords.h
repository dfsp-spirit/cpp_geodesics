#pragma once

#include "libfs.h"
#include "typedef_vcg.h"
#include <vcg/complex/complex.h>
#include <vcg/complex/append.h>
#include <vcg/container/simple_temporary_data.h>
#include <vcg/space/point3.h>


/// @brief Get mesh vertex coords as `nx3` 2D vector of floats.
std::vector<std::vector<float>> mesh_vertex_coords(MyMesh& m) {
  SimpleTempData<MyMesh::VertContainer,int> vert_indices(m.vert);

  std::vector<std::vector<float>> vertex_coords = std::vector<std::vector<float>>(m.vn, std::vector<float>(3, 0.0));;

  VertexIterator vi = m.vert.begin();
  for (int i=0; i < m.vn; i++) {
    vert_indices[vi] = i;
    vertex_coords[i][0] = ((vi)->P().X());
    vertex_coords[i][1] = ((vi)->P().Y());
    vertex_coords[i][2] = ((vi)->P().Z());
    ++vi;
  }
  return vertex_coords;
}
