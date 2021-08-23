#include "typedef_vcg.h"
#include <vcg/complex/complex.h>
#include <vcg/complex/append.h>
#include <vcg/container/simple_temporary_data.h>


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>


std::vector<std::vector<int>> mesh_adj(MyMesh& m, std::vector<int> query_vertices, int numstep=1, bool include_self=false) {
  m.vert.EnableVFAdjacency();
  m.face.EnableFFAdjacency();
  m.face.EnableVFAdjacency();

  tri::UpdateTopology<MyMesh>::FaceFace(m);
  tri::UpdateTopology<MyMesh>::VertexFace(m);
  FaceIterator fi;
  VertexIterator vi;

  // Create int vertex indices to return.
  SimpleTempData<MyMesh::VertContainer,int> indices(m.vert);
  vi = m.vert.begin();
  for (int i=0; i < m.vn; i++) {
    indices[vi] = i;
    ++vi;
  }

  // Start neighborhood computation
  std::vector<std::vector<int>> neighborhoods;
  for(size_t i=0; i < query_vertices.size(); ++i) {
    int qv = query_vertices[i];
    vi = m.vert.begin()+qv;
    MyVertex* pqv = &*vi;
    std::vector<MyVertex*> neigh;
    vcg::face::VVExtendedStarVF<MyFace>(pqv, numstep, neigh);

    // Collect indices of vertices.
    std::vector<int> neighidx;
    if(include_self) {
      neighidx.push_back(qv);
    }
    for(size_t j=0; j<neigh.size(); j++) {
      neighidx.push_back(indices[neigh[j]]);
    }
    neighborhoods.push_back(neighidx);
  }

  return neighborhoods;
}


/// Dont roll your own JSON, they told us.
std::string neigh_to_json(std::vector<std::vector<int>> neigh) {
    std::stringstream is;
    is << "{\n";
    for(size_t i=0; i < neigh.size(); i++) {
        is << "  \"" << i << "\": [";
        for(size_t j=0; j < neigh[i].size(); j++) {
            is << " " << neigh[i][j];
            if(j < neigh[i].size()-1) {
                is << ",";
            }
        }
        is << " ]";
        if(i < neigh.size()-1) {
            is <<",";    
        }
        is <<"\n";
    }
    is << "}\n";
    return is.str();
}


void strtofile(std::string outstring, std::string filename) {
    std::ofstream outs(filename);
    if(outs.is_open()) {
        outs << outstring;
        outs.close();
    } else {
        std::cerr << "Failed to open output file '" << filename << "'.\n";
    }
}
