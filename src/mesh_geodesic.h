
#include "typedef_vcg.h"
#include <vcg/complex/complex.h>
#include <vcg/complex/append.h>
#include<vcg/complex/algorithms/geodesic.h>
#include <vcg/container/simple_temporary_data.h>


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>


// Compute pseudo-geodesic distance from query vertices 'verts' to all others (or to those
// within a maximal distance of maxdist_ if it is > 0).
std::vector<float> geodist(MyMesh m, std::vector<int> verts, float maxdist) {
  
    int n = verts.size();
    VertexIterator vi;
    FaceIterator fi;

    // Setup mesh
    m.vert.EnableVFAdjacency();
    m.vert.EnableQuality();
    m.face.EnableFFAdjacency();
    m.face.EnableVFAdjacency();
    tri::UpdateTopology<MyMesh>::VertexFace(m);

    // Prepare seed vector
    std::vector<MyVertex*> seedVec;
    for (int i=0; i < n; i++) {
      vi = m.vert.begin()+verts[i];
      seedVec.push_back(&*vi);
    }

    // Compute pseudo-geodesic distance by summing dists along shortest path in graph.
    tri::EuclideanDistance<MyMesh> ed;
    if(maxdist > 0.0) {
      tri::Geodesic<MyMesh>::PerVertexDijkstraCompute(m,seedVec,ed,maxdist);
    } else {
      tri::Geodesic<MyMesh>::PerVertexDijkstraCompute(m,seedVec,ed);
    }

    std::vector<float> geodists;
    vi=m.vert.begin();
    for (int i=0; i < m.vn; i++) {
      geodists.push_back(vi->Q());
      ++vi;
    }
    return geodists;
}
