#pragma once

#include "libfs.h"
#include "typedef_vcg.h"
#include <vcg/complex/complex.h>
#include <vcg/complex/append.h>
#include <vcg/container/simple_temporary_data.h>
#include <vcg/space/point3.h>


/// @brief Create a VCGLIB MyMesh instance from an fs::Mesh
/// @param m pointer to an empty MyMesh instance
/// @param fs_surface the fs::Mesh instance to convert
void vcgmesh_from_fs_surface(MyMesh* m, const fs::Mesh& fs_surface) {
  int nv = fs_surface.num_vertices();
  int nf = fs_surface.num_faces();

  // Add vertices
  vcg::tri::Allocator<MyMesh>::AddVertices(*m, nv);

  //std::cout << " Creating MyMesh instance with " << nv << " vertices and " << nf << " faces.\n";

  // Create vertex pointers, used later when creating faces.
  std::vector<MyMesh::VertexPointer> ivp;
  ivp.resize(nv);

  // Set vertex coordinates.
  for (int i=0; i < nv; i++) {
    VertexIterator vi = m->vert.begin()+i;
    ivp[i]=&*vi;
    (*vi).P() = CoordType(fs_surface.vm_at(i, 0), fs_surface.vm_at(i, 1), fs_surface.vm_at(i, 2));
  }

  // Create faces
  vcg::tri::Allocator<MyMesh>::AddFaces(*m, nf);
  for (int i=0; i < nf; i++) {
	  FaceIterator fi=m->face.begin()+i;
	  for (int j = 0; j < 3; j++)  {
		  (*fi).V(j)=ivp[fs_surface.fm_at(i, j)];
    }
  }
}


/// Create an fs::Mesh instance from a VCGLIB MyMesh
void fs_surface_from_vcgmesh(fs::Mesh* surf, MyMesh& m) {
  SimpleTempData<MyMesh::VertContainer,int> vert_indices(m.vert);

  std::vector<float> vertex_coords;

  VertexIterator vi = m.vert.begin();
  for (int i=0; i < m.vn; i++) {
    vert_indices[vi] = i;
    vertex_coords.push_back((vi)->P().X());
    vertex_coords.push_back((vi)->P().Y());
    vertex_coords.push_back((vi)->P().Z());
    ++vi;
  }

  std::vector<int> faces; // Their vertex indices.

  FaceIterator fi = m.face.begin();
  for (int i=0; i < m.fn; i++) {
    faces.push_back(vert_indices[fi->V(0)]);
    faces.push_back(vert_indices[fi->V(1)]);
    faces.push_back(vert_indices[fi->V(2)]);
    ++fi;
  }

  surf->vertices = vertex_coords;
  surf->faces = faces;
}


