#include "libfs.h"
#include "typedef_vcg.h"
#include "mesh_area.h"
#include <vcg/complex/complex.h>
#include <vcg/complex/append.h>
#include<vcg/complex/algorithms/geodesic.h>
#include <vcg/container/simple_temporary_data.h>

#define _USE_MATH_DEFINES
#include <cmath>


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>


// Compute pseudo-geodesic distance from query vertices 'verts' to all others (or to those
// within a maximal distance of maxdist_ if it is > 0). Often 'verts' only contains a single source vertex.
std::vector<float> geodist(MyMesh& m, std::vector<int> verts, float maxdist) {
  
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


/// Compute for each mesh vertex the mean geodesic distance to all others, parallel using OpenMP.
std::vector<float> mean_geodist_p(MyMesh &m) {
  
  // The MyMesh instance cannot be shared between the processes because it
  // gets changed when the geodist function is run (distances are stored in
  // the vertices' quality field). Also, firstprivate(m) does not work because
  // it has no copy constructor. We therefore convert it to an fs::Mesh
  // and share that, then constuct one MyMesh instance per thread in the
  // parallel for loop from the shared fs::Mesh.
  // I guess an alternative could be to wrap the MyMesh in our own wrapper class,
  // provide a copy constructor for that which copies the member MyMesh using the
  // VCGLIB mesh copy functionality, use firstprivate() on the wrapper, and then pass
  // in each thread the inner MyMesh from the wrapper to the geodist function. We have
  // not tried this though. 
  fs::Mesh surf;
  fs_surface_from_vcgmesh(&surf, m);

  std::vector<float> meandists;
  size_t nv = surf.num_vertices();
  float max_dist = -1.0;
  meandists.resize(nv);  
  
  # pragma omp parallel for firstprivate(max_dist) shared(surf)
  for(size_t i=0; i<nv; i++) {
    MyMesh m;
    vcgmesh_from_fs_surface(&m, surf);
    std::vector<int> query_vert;
    query_vert.resize(1);
    query_vert[0] = i;
    std::vector<float> gdists = geodist(m, query_vert, max_dist);
    double dist_sum = 0.0;
    for(size_t j=0; j<nv; j++) {
        dist_sum += gdists[j];
    }
    meandists[i] = (float)(dist_sum / nv);
  }
  return meandists;
}


/// Compute for each mesh vertex the mean geodesic distance to all others, sequentially.
std::vector<float> mean_geodist(MyMesh &m) {
  std::vector<float> meandists;
  size_t nv = m.VN();
  float max_dist = -1.0;
  meandists.resize(nv);
  
  for(size_t i=0; i<nv; i++) {
    std::vector<int> query_vert;
    query_vert.resize(1);
    query_vert[0] = i;
    std::vector<float> gdists = geodist(m, query_vert, max_dist);
    double dist_sum = 0.0;
    for(size_t j=0; j<nv; j++) {
        dist_sum += gdists[j];
    }
    meandists[i] = (float)(dist_sum / nv);
  }
  return meandists;
}


/// Compute geodesic circles at each query vertex and return their radius and perimeter (and mean geodesic distance if requested).
std::vector<std::vector<float>> geodesic_circles(MyMesh& m, std::vector<int> query_vertices, float scale=5.0, bool do_meandist=false) {
  float sampling = 10.0;
  double mesh_area = mesh_area_total(m);  
  double area_scale = (scale * mesh_area) / 100.0;
  double r_cycle = sqrt(area_scale / M_PI);
  
  double max_dist = r_cycle + 10.0; // Early termination of geodesic distance computation for dramatic speed-up.
  if(do_meandist) {
    max_dist = -1.0; // Compute full pairwise geodesic distances if requested.
  }

  // Use all vertices if query_vertices is empty.
  if(query_vertices.empty()) {
    for(int i=0; i<m.vn; i++) {
      query_vertices.push_back(i);
    }
  }

  // TODO: compute here
  std::vector<std::vector<float>> fake_res;
  return fake_res;
}


