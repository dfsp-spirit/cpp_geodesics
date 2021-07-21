#pragma once

#include "libfs.h"
#include "spline.h"

#include "typedef_vcg.h"
#include "mesh_area.h"
#include <vcg/complex/complex.h>
#include <vcg/complex/append.h>
#include <vcg/complex/algorithms/geodesic.h>
#include <vcg/container/simple_temporary_data.h>

#define _USE_MATH_DEFINES
#include <cmath>


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <cassert>


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
    std::vector<MyVertex*> seedVec(verts.size());
    for (int i=0; i < n; i++) {
      vi = m.vert.begin()+verts[i];
      seedVec[i] = &*vi;
    }

    // Compute pseudo-geodesic distance by summing dists along shortest path in graph.
    tri::EuclideanDistance<MyMesh> ed;
    if(maxdist > 0.0) {
      tri::Geodesic<MyMesh>::PerVertexDijkstraCompute(m,seedVec,ed,maxdist);
    } else {
      tri::Geodesic<MyMesh>::PerVertexDijkstraCompute(m,seedVec,ed);
    }

    std::vector<float> geodists(m.vn);
    vi=m.vert.begin();
    for (int i=0; i < m.vn; i++) {
      geodists[i] = vi->Q();
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


/// A linspace or seq function for C++.
template<typename T>
std::vector<double> linspace(T start_in, T end_in, int num_in) {

  std::vector<double> linspaced;

  double start = static_cast<double>(start_in);
  double end = static_cast<double>(end_in);
  double num = static_cast<double>(num_in);

  if (num == 0) { return linspaced; }
  if (num == 1) 
    {
      linspaced.push_back(start);
      return linspaced;
    }

  double delta = (end - start) / (num - 1);

  for(int i=0; i < num-1; ++i)
    {
      linspaced.push_back(start + delta * i);
    }
  linspaced.push_back(end); // I want to ensure that start and end
                            // are exactly the same as the input
  return linspaced;
}


///  Compute geodesic circle area and perimeter at location defined by geodists for all radii.
///  The location at which it will be computed is the vertex for which the geodesic distances were computed.
///
/// This function is internal, it is called by geodesic_circles().
std::vector<std::vector<double>> _compute_geodesic_circle_stats(MyMesh& m, std::vector<float> geodist, std::vector<double> sample_at_radii) {

  fs::Mesh surf;
  fs_surface_from_vcgmesh(&surf, m);

  int nr = sample_at_radii.size();
  std::vector<double> per_face_area = mesh_area_per_face(m);
  for(int radius_idx=0; radius_idx<nr; radius_idx++) {
    double radius = sample_at_radii[radius_idx];

    ///////////////////// WARNING: We have to double-check what the distance values is for vertices for which NO values was computed.
    /////////////////////          If it is 0.0 instead of NA or INF, the following code will not work and we need to replace the values first.

    std::vector<bool> vert_in_radius(m.vn); // Whether vertex v is in radius, i.e., whether its geodesic distance value is < radius.
    for(int i=0; i<m.vn; i++) {
      vert_in_radius[i] = geodist[i] < radius;
    }

    // Count how many vertices per face are in radius.
    std::vector<int> faces_num_verts_in_radius(m.fn);
    for(int i=0; i<m.fn; i++) {
      faces_num_verts_in_radius[i] = 0;
      for(int j=0; j<3; j++) {
        if(geodist[surf.fm_at(i, j)] < radius) {
          faces_num_verts_in_radius[i]++;
        }
      }
    }

    double total_area_in_radius = 0.0; // So far.
    double total_perimeter = 0.0;

    // Add the area of all faces which are full in radius (all 3 vertices in radius).
    for(int i=0; i<m.fn; i++) {
      if(faces_num_verts_in_radius[i] == 3) {
        total_area_in_radius += per_face_area[i];
      }
    }

    // Now compute partial area for faces which are only partly in range.
    for(int i=0; i<m.fn; i++) {
      if(faces_num_verts_in_radius[i] != 3) {
        int num_verts_in_radius = faces_num_verts_in_radius[i];
        int k = -1;
        std::vector<int> face_verts = surf.face_vertices(i);
        if(num_verts_in_radius == 2) { // 2 in, 1 out
          for(int j=0; j<3; j++) {
            if(vert_in_radius[face_verts[j]] == false) {
              k=j;
            }
          }          
        } else { // 1 in, 2 out
          for(int j=0; j<3; j++) {
            if(vert_in_radius[face_verts[j]] == true) {
              k=j;
            }
          }
        }
        assert(k>=0);
      }
    }

  }

  std::vector<std::vector<double>> fake_res;
  return fake_res;
}


/// Compute geodesic circles at each query vertex and return their radius and perimeter (and mean geodesic distance if requested).
/// If 'query_vertices' is empty, this function will work on ALL vertices.
std::vector<std::vector<float>> geodesic_circles(MyMesh& m, std::vector<int> query_vertices, float scale=5.0, bool do_meandist=false) {
  double sampling = 10.0;
  double mesh_area = mesh_area_total(m);  
  double area_scale = (scale * mesh_area) / 100.0;
  double r_cycle = sqrt(area_scale / M_PI);
  
  double max_dist = r_cycle + 10.0; // Early termination of geodesic distance computation for dramatic speed-up.
  if(do_meandist) {
    max_dist = -1.0; // Compute full pairwise geodesic distances if requested.
  }

  // Use all vertices if query_vertices is empty.
  if(query_vertices.empty()) {
    query_vertices.resize(m.vn);
    for(int i=0; i<m.vn; i++) {
      query_vertices[i] = i;
    }
  }

  std::vector<float> radius, perimeter, meandist;
  size_t nqv = query_vertices.size();
  radius.resize(nqv);
  perimeter.resize(nqv);
  meandist.resize(nqv);
  for(size_t i=0; i<nqv; i++) {
    int qv = query_vertices[i];
    std::vector<int> query_vertex(1);
    query_vertex[0] = qv;
    std::vector<float> v_geodist = geodist(m, query_vertex, max_dist);
    if(do_meandist) {
      meandist[i] = std::accumulate(v_geodist.begin(), v_geodist.end(), 0.0) / v_geodist.size(); 
    }

    std::vector<double> sample_at_radii = linspace<double>(r_cycle-10.0, r_cycle+10.0, sampling);
    std::vector<std::vector<double>> circle_stats = _compute_geodesic_circle_stats(m, v_geodist, sample_at_radii);
    std::vector<double> circle_areas = circle_stats[0];
    std::vector<double> circle_perimeters = circle_stats[1];

    std::vector<double> x = linspace<double>(1.0, sampling, 1.0); // spline x values
    std::vector<double> xx = linspace<double>(1.0, sampling, 0.1);  // where to sample
    int num_samples = xx.size();

    // Create cubic splines interpolation.
    tk::spline spl_areas(x, circle_areas);
    tk::spline spl_radius(x, sample_at_radii);
    tk::spline spl_perimeters(x, circle_perimeters);
    // Get interpolated values.
    std::vector<double> sampled_areas(num_samples);
    for(int i=0; i<num_samples; i++) { sampled_areas[i] = spl_areas(xx[i]); }
    std::vector<double> sampled_radii(num_samples);
    for(int i=0; i<num_samples; i++) { sampled_radii[i] = spl_radius(xx[i]); }
    std::vector<double> sampled_perimeters(num_samples);
    for(int i=0; i<num_samples; i++) { sampled_perimeters[i] = spl_perimeters(xx[i]); }

    // Determine index of min
    for(int i=0; i<num_samples; i++) {
      sampled_areas[i] = fabs(area_scale - sampled_areas[i]);
    }
    int min_index = std::distance(sampled_areas.begin(),std::min_element(sampled_areas.begin(),sampled_areas.end()));
    // Collect results.
    radius[i] = sampled_radii[min_index];
    perimeter[i] = sampled_perimeters[min_index];
  }
  
  // Prepare and return results.
  std::vector<std::vector<float>> res;
  res.push_back(radius);
  res.push_back(perimeter);
  if(do_meandist) {
    res.push_back(meandist);    
  }
  return res;
}


