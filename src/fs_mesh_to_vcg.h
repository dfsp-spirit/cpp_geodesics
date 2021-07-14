#pragma once

#include "libfs.h"
#include "typedef_vcg.h"


void vcgmesh_from_fs_surface(MyMesh* m, const fs::Mesh& fs_surface) {  
  int nv = fs_surface.num_vertices();
  int nf = fs_surface.num_faces();
  MyMesh::VertexPointer ivp[3];
  
  MyMesh::VertexIterator vi = vcg::tri::Allocator<MyMesh>::AddVertices(*m, nv);
  MyMesh::FaceIterator fi = vcg::tri::Allocator<MyMesh>::AddFaces(*m, nf);

  std::cout << "Creating MyMesh instance with " << nv << " vertices and " << nf << " faces.\n";

  int face_vertices[3];
  for(int face_idx=0; face_idx<nf; face_idx++) {
    // Determine the vertices belonging to this face.
    face_vertices[0] = fs_surface.fm_at(face_idx, 0);
    face_vertices[1] = fs_surface.fm_at(face_idx, 1);
    face_vertices[2] = fs_surface.fm_at(face_idx, 2);
    if(face_idx == 0) {
      std::cout << "Adding face #" << face_idx << " consisting of vertices " << face_vertices[0] << ", " << face_vertices[1] << ", " << face_vertices[2] << ".\n";
      std::cout << "Coordinates for the first vertex of the face are x=" << fs_surface.vm_at(face_vertices[0], 0) << ", y=" << fs_surface.vm_at(face_vertices[0], 1) << ", z=" << fs_surface.vm_at(face_vertices[0], 2) << ".\n";
    }

    // THIS IS A BUG! We need to set the coords for the proper vertex index. Simply incrementing vi at the end of the next 3 statements is wrong, as a vertex appears in several faces, vi will be wrong and at some point overflow!
    // We could look at RvcgIO.h for an example of building a mesh.
    ivp[0]=&*vi; vi->P()=MyMesh::CoordType (fs_surface.vm_at(face_vertices[0], 0), fs_surface.vm_at(face_vertices[0], 1), fs_surface.vm_at(face_vertices[0], 2)); ++vi;
    ivp[1]=&*vi; vi->P()=MyMesh::CoordType (fs_surface.vm_at(face_vertices[1], 0), fs_surface.vm_at(face_vertices[1], 1), fs_surface.vm_at(face_vertices[1], 2)); ++vi;
    ivp[2]=&*vi; vi->P()=MyMesh::CoordType (fs_surface.vm_at(face_vertices[2], 0), fs_surface.vm_at(face_vertices[2], 1), fs_surface.vm_at(face_vertices[2], 2)); ++vi;


    // Add the 3 vertices (with their x,y,z coords) and store the vertex pointers.
    //for(int i=0; i<3; i++) {
    //  ivp[i] = &*vcg::tri::Allocator<MyMesh>::AddVertex(*m,MyMesh::CoordType ( fs_surface.vm_at(face_vertices[i], 0), fs_surface.vm_at(face_vertices[i], 1), fs_surface.vm_at(face_vertices[i], 2)));
    //}
    // Add the face consisting of the 3 vertices.
    // vcg::tri::Allocator<MyMesh>::AddFace(*m, ivp[0],ivp[1],ivp[2]);

    if(face_idx <= 10) {
      std::cout << "Creating face " << face_idx << ".\n";
    }

    fi->V(0)=ivp[0];
    if(face_idx == 0) {
      std::cout << " -1st done.\n";
    }
    fi->V(1)=ivp[1];
    if(face_idx == 0) {
      std::cout << " -2nd done.\n";
    }
    fi->V(2)=ivp[2];
    if(face_idx == 0) {
      std::cout << " -3rd done.\n";
    }
    ++fi;
    if(face_idx == 0) {
      std::cout << "-Face pointer incremented.\n";
    }
  }
}