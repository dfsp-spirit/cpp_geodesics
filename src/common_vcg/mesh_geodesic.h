#pragma once

#include "libfs.h"
#include "spline.h"

#include "typedef_vcg.h"
#include "mesh_area.h"
#include "mesh_edges.h"
#include "vec_math.h"

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

  # pragma omp parallel for firstprivate(max_dist) shared(surf, meandists)
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

struct GeodNeighbor {
  GeodNeighbor() : index(0), distance(0.0) {}
  GeodNeighbor(size_t index, float distance) : index(index), distance(distance) {}
  size_t index; // The index of the neighbor vertex.
  float distance; // The geodesic distance to that neighbor.
  // TODO: add vertex normal field here, and compute with vcglib.
};


/// @brief Struct modeling a spatial neighborhood with coordinates, and geodesic or Euclidean distances of neighbors to central vertex.
///
/// @details The coordinates of the neighbors should be
/// centered on the central/source vertex (i.e., it should be at (0,0,0)) but
/// NOT scaled (e.g., to range 0..1).
/// Note that the fact that the central vertex is the origin means that we do
/// not need to store its coordinates. If you need that (and do not center), you
/// will have to add the vertex itself to its neighborhood.
struct Neighborhood {
  Neighborhood(size_t index, std::vector<std::vector<float>> coords, std::vector<float> distances) : index(index), coords(coords), distances(distances) {}
  Neighborhood(size_t index, std::vector<std::vector<float>> coords) : index(index), coords(coords), distances(std::vector<float>(coords.size(), 0.0)) {}
  Neighborhood() : index(0), coords(std::vector<std::vector<float>>(0)), distances(std::vector<float>(0)) {}
  size_t index; ///< The index of the central/source vertex.
  std::vector<std::vector<float> > coords; ///< 2D array of coordinates, for n 3D coordinates this will have dimensions (n, 3).
  std::vector<float> distances;  ///< distances from the central vertex -- these can be Euclidean (easily derivable from coords) or Geodesic.
  //std::vector<float> normals; // TODO: add vertex normal field here, and compute with vcglib.
};

/// @brief Compute vertex neighborhoods: for a source vertex, compute centered coordinates of all given neighbors.
/// @details The distances in the return value are geodesic distances.
/// @param geod_neighbors: (n, m) 2D vector of `GeodNeighbor`, typically the neighborhoods (each consisting of `m` neighbors) for all `n` vertices of some mesh. Neighbors are encoded as vertex indices in the GeodNeighbor struct.
/// @param mesh: the mesh, used to get the vertex coordinates from the vertex indices in geod_neighbors.
/// @return vector of `n` Neighborhood instances
std::vector<Neighborhood> neighborhoods_from_geod_neighbors(const std::vector<std::vector<GeodNeighbor> > geod_neighbors, MyMesh &mesh) {
  size_t num_neighborhoods = geod_neighbors.size();
  std::cout << "Computing neighborhoods for " << num_neighborhoods << " vertices and their geodesic neighbors." << "\n";
  std::vector<Neighborhood> neighborhoods;
  size_t neigh_size;
  std::vector<std::vector<float>> neigh_coords;
  std::vector<float> neigh_distances;
  std::vector<float> source_vert_coords;
  std::vector<int> neigh_indices;
  size_t neigh_mesh_idx;
  for(size_t i = 0; i < num_neighborhoods; i++) {
    neigh_size = geod_neighbors[i].size();
    neigh_indices = std::vector<int>(neigh_size);
    neigh_distances = std::vector<float>(neigh_size);
    neigh_coords = std::vector<std::vector<float> >(neigh_size, std::vector<float> (3, 0.0));
    for(size_t j = 0; j < neigh_size; j++) {
      neigh_mesh_idx = geod_neighbors[i][j].index;
      neigh_indices[j] = neigh_mesh_idx;
      neigh_distances[j] = geod_neighbors[i][j].distance;  // This is the geodesic distance in this case!
      neigh_coords[j] = std::vector<float> {mesh.vert[neigh_mesh_idx].P().X(), mesh.vert[neigh_mesh_idx].P().X(), mesh.vert[neigh_mesh_idx].P().Z()};
      source_vert_coords = std::vector<float> {mesh.vert[i].P().X(), mesh.vert[i].P().Y(), mesh.vert[i].P().Z()};
      // Center the coords around source vertex (make it the origin):
      for(size_t k = 0; k < 3; k++) {
        neigh_coords[i][k] -= source_vert_coords[k];
      }
    }
    neighborhoods.push_back(Neighborhood(i, neigh_coords, neigh_distances));
  }
  return neighborhoods;
}

/// @brief Computes neighborhoods where the distance is the geodesic distance.
/// @details The distances in the return value are Euclidean distances.
std::vector<Neighborhood> neighborhoods_from_edge_neighbors(const std::vector<std::vector<int> > edge_neighbors, MyMesh &mesh) {
  size_t num_neighborhoods = edge_neighbors.size();
  std::vector<Neighborhood> neighborhoods;
  size_t neigh_size, neigh_mesh_idx;
  std::vector<std::vector<float>> neigh_coords;
  std::vector<float> neigh_distances;
  std::vector<float> source_vert_coords;
  std::vector<int> neigh_indices;
  for(size_t i = 0; i < num_neighborhoods; i++) {
    neigh_size = edge_neighbors[i].size();
    neigh_indices = std::vector<int>(neigh_size);
    neigh_distances = std::vector<float>(neigh_size);
    neigh_coords = std::vector<std::vector<float> >(neigh_size, std::vector<float> (3, 0.0));
    for(size_t j = 0; j < neigh_size; j++) {
      neigh_mesh_idx = edge_neighbors[i][j];
      neigh_indices[j] = neigh_mesh_idx;
      neigh_coords[i] = std::vector<float> {mesh.vert[neigh_mesh_idx].P().X(), mesh.vert[neigh_mesh_idx].P().X(), mesh.vert[neigh_mesh_idx].P().Z()};
      source_vert_coords = std::vector<float> {mesh.vert[i].P().X(), mesh.vert[i].P().Y(), mesh.vert[i].P().Z()};
      neigh_distances[j] = dist_euclid(neigh_coords[i], source_vert_coords); // This is the Euclidean distance in this case!
      // Center the coords around source vertex (make it the origin):
      for(size_t k = 0; k < 3; k++) {
        neigh_coords[i][k] -= source_vert_coords[k];
      }
    }
    neighborhoods.push_back(Neighborhood(i, neigh_coords, neigh_distances));
  }
  return neighborhoods;
}

/// @brief Compute for each mesh vertex all vertices in a given distance (and that distance), parallel using OpenMP.
std::vector<std::vector<GeodNeighbor>> geod_neighborhood(MyMesh &m, const float max_dist = 5.0, const bool include_self = true) {

  // The MyMesh instance cannot be shared between the processes because it
  // gets changed when the geodist function is run (distances are stored in
  // the vertices' quality field). See the comment in mean_geodist_p() for
  // details.
  fs::Mesh surf;
  fs_surface_from_vcgmesh(&surf, m);

  size_t nv = surf.num_vertices();
  std::vector<std::vector<GeodNeighbor>> neighborhoods(nv, std::vector<GeodNeighbor>());

  # pragma omp parallel for firstprivate(max_dist) shared(surf, neighborhoods)
  for(size_t i=0; i<nv; i++) {
    MyMesh m;
    vcgmesh_from_fs_surface(&m, surf);
    std::vector<int> query_vert= {(int)i};
    std::vector<float> gdists = geodist(m, query_vert, max_dist);

    for(size_t j=0; j<gdists.size(); j++) {
      if(i == j) {
        if(include_self) {
          neighborhoods[i].push_back(GeodNeighbor(j, 0.0)); // The vertex itself (in distance 0).
        }
      } else {
        if(gdists[j] > 0.0 && gdists[j] <= max_dist) {
          neighborhoods[i].push_back(GeodNeighbor(j, gdists[j]));
        }
      }
    }

  }
  return neighborhoods;
}


/// Dont roll your own JSON, they told us.
std::string geod_neigh_to_json(std::vector<std::vector<GeodNeighbor>> neigh) {
    std::stringstream is;
    is << "{\n";
    is << "  \"neighbors\": {\n";
    for(size_t i=0; i < neigh.size(); i++) {
        is << "  \"" << i << "\": [";
        for(size_t j=0; j < neigh[i].size(); j++) {
            is << " " << neigh[i][j].index;
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
    is << "  },\n";
    is << "  \"distances\": {\n";
    for(size_t i=0; i < neigh.size(); i++) {
        is << "  \"" << i << "\": [";
        for(size_t j=0; j < neigh[i].size(); j++) {
            is << " " << neigh[i][j].distance;
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
    is << "  }\n";
    is << "}\n";
    return is.str();
}


/// Who would write his own CSV exporter in 2022?!
std::string geod_neigh_to_csv(const std::vector<std::vector<GeodNeighbor>> neigh, const std::string sep=",") {
    std::stringstream is;
    is << "source" << sep << "target" << sep << "distance" << "\n";

    for(size_t i=0; i < neigh.size(); i++) {
        for(size_t j=0; j < neigh[i].size(); j++) {
            is << i << sep << neigh[i][j].index << sep << neigh[i][j].distance << "\n";
        }
    }
    return is.str();
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


/// Assumes the last value is in.
template<typename T>
int numsteps_for_stepsize(T start_in, T end_in, double stepsize) {
  double start = static_cast<double>(start_in);
  double end = static_cast<double>(end_in);
  double delta = end - start;
  int numsteps = (int)ceil(delta / stepsize);
  return numsteps + 1;
}

/// A linspace or seq function for C++.
template<typename T>
std::vector<double> linspace(T start_in, T end_in, int num_in) {

  std::vector<double> linspaced;

  double start = static_cast<double>(start_in);
  double end = static_cast<double>(end_in);
  double num = static_cast<double>(num_in);

  if (num == 0) { return linspaced; }
  if (num == 1) {
      linspaced.push_back(start);
      return linspaced;
  }

  double delta = (end - start) / (num - 1);

  for(int i=0; i < num-1; ++i) {
      linspaced.push_back(start + delta * i);
  }
  linspaced.push_back(end); // Ensure that start and end are exactly the same as the input.
  return linspaced;
}


///  Compute geodesic circle area and perimeter at location defined by geodists for all radii.
///  The location at which it will be computed is the vertex for which the geodesic distances were computed.
///
/// This function is internal, it is called by geodesic_circles().
std::vector<std::vector<double>> _compute_geodesic_circle_stats(MyMesh& m, std::vector<float> geodist, std::vector<double> sample_at_radii) {

  fs::Mesh surf;
  fs_surface_from_vcgmesh(&surf, m);
  float max_possible_float = std::numeric_limits<float>::max();

  int nr = sample_at_radii.size();
  std::vector<double> per_face_area = mesh_area_per_face(m);

  std::vector<double> areas_by_radius(nr);
  std::vector<double> perimeters_by_radius(nr);

  for(int radius_idx=0; radius_idx<nr; radius_idx++) {
    double radius = sample_at_radii[radius_idx];

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
      if(faces_num_verts_in_radius[i] != 3 && faces_num_verts_in_radius[i] != 0) {
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
        // Reorder vertex indices of face, based on k.
        std::vector<int> face_verts_copy = surf.face_vertices(i); // tmp
        if(k == 1) {
          face_verts[0] = face_verts_copy[1];
          face_verts[1] = face_verts_copy[2];
          face_verts[2] = face_verts_copy[0];
        } else if(k==2) {
          face_verts[0] = face_verts_copy[2];
          face_verts[1] = face_verts_copy[0];
          face_verts[2] = face_verts_copy[1];
        } // No re-ordering for k==0.

        std::vector<float> face_vertex_dists(3);  // Get distances for all vertices of this face.
        face_vertex_dists[0] = geodist[face_verts[0]] - radius;
        face_vertex_dists[1] = geodist[face_verts[1]] - radius;
        face_vertex_dists[2] = geodist[face_verts[2]] - radius;

        // If these asserts fail, the extra_dist added to the radius to create max_dist in the geodesic_circles() function is too small.
        assert(geodist[face_verts[0]] < (max_possible_float - 0.01));
        assert(geodist[face_verts[1]] < (max_possible_float - 0.01));
        assert(geodist[face_verts[2]] < (max_possible_float - 0.01));

        // The following 3 vectors should be a matrix.
        std::vector<float> coords_v0 = surf.vertex_coords(face_verts[0]);
        std::vector<float> coords_v1 = surf.vertex_coords(face_verts[1]);
        std::vector<float> coords_v2 = surf.vertex_coords(face_verts[2]);

        // These computations use vector math with overloaded operators from vec_math.h
        float alpha1 = face_vertex_dists[1]/(face_vertex_dists[1]-face_vertex_dists[0]);
        std::vector<float> v1 = alpha1 * coords_v0 + (1.0f-alpha1) * coords_v1;
        float alpha2 = face_vertex_dists[2]/(face_vertex_dists[2]-face_vertex_dists[0]);
        std::vector<float> v2 = alpha2 * coords_v0 + (1.0f-alpha2) * coords_v2;

        float b = vnorm(cross(coords_v0 - v1, coords_v0 - v2)) / 2.0;
        if(num_verts_in_radius == 2) { // 2 in, 1 out
          total_area_in_radius += per_face_area[i] - b;
        } else { // 1 in, 2 out
          total_area_in_radius += b;
        }

        total_perimeter += vnorm(v1 - v2);
      }

    }
    // Collect results
    areas_by_radius[radius_idx] = total_area_in_radius;
    perimeters_by_radius[radius_idx] = total_perimeter;
  }

  std::vector<std::vector<double>> res;
  res.push_back(areas_by_radius);
  res.push_back(perimeters_by_radius);
  return res;
}


/// Compute geodesic circles at each query vertex and return their radius and perimeter (and mean geodesic distance if requested).
/// If 'query_vertices' is empty, this function will work on ALL vertices.
/// If 'do_meandist' is true, this function will compute the mean geodesic distances to all other vertices for each vertex and
/// return those as well. This is only partially needed for the function (it only needs to know for each vertex the geodesic
/// distances in a certain radius, not to ALL vertices), but it is faster to do it here instead of separately computing the mean
/// distances with another function call to mean_geodist_p()/mean_geodist() IF you need them anyways. If in doubt, leave this
/// disabled for a dramatic speedup (how much depends on the 'scale' parameter).
std::vector<std::vector<float>> geodesic_circles(MyMesh& m, std::vector<int> query_vertices, float scale=5.0, bool do_meandist=false) {

  double sampling = 10.0;
  double mesh_area = mesh_area_total(m);
  double area_scale = (scale * mesh_area) / 100.0;
  double r_cycle = sqrt(area_scale / M_PI);
  float max_possible_float = std::numeric_limits<float>::max();

  std::vector<double> edge_lengths = mesh_edge_lengths(m);
  double mean_len = std::accumulate(edge_lengths.begin(), edge_lengths.end(), 0.0) / (double)edge_lengths.size();
  double max_edge_len = *std::max_element(edge_lengths.begin(), edge_lengths.end());
  std::cout  << "     o Mesh has " << edge_lengths.size() << " edges with average length " << mean_len << " and maximal length " << max_edge_len << ".\n";

  double extra_dist = max_edge_len * 8.0;
  double max_dist = r_cycle + extra_dist; // Early termination of geodesic distance computation for dramatic speed-up.
  if(do_meandist) {
    max_dist = -1.0; // Compute full pairwise geodesic distances if meandist computation was requested.
  } else {
    std::cout  << "     o Using extra_dist=" << extra_dist << ", resulting in max_dist=" << max_dist << ".\n";
  }

  // Use all vertices if query_vertices is empty.
  if(query_vertices.empty()) {
    query_vertices.resize(m.vn);
    for(int i=0; i<m.vn; i++) {
      query_vertices[i] = i;
    }
  }
  //std::cout  << "Using " << query_vertices.size() << " query vertices.\n";

  std::vector<float> radius, perimeter, meandist;
  size_t nqv = query_vertices.size();
  radius.resize(nqv);
  perimeter.resize(nqv);
  meandist.resize(nqv);

  fs::Mesh surf;  // Create FreeSurfer mesh that is copyable for OpenMP.
  fs_surface_from_vcgmesh(&surf, m);

  # pragma omp parallel for shared(surf, radius, perimeter, meandist)
  for(size_t i=0; i<nqv; i++) {
    MyMesh mt; // per thread, recreated from FreeSurfer mesh
    vcgmesh_from_fs_surface(&mt, surf);
    int qv = query_vertices[i];
    std::vector<int> query_vertex = { qv };
    std::vector<float> v_geodist = geodist(mt, query_vertex, max_dist);

    if(do_meandist) {
      meandist[i] = std::accumulate(v_geodist.begin(), v_geodist.end(), 0.0) / (float)v_geodist.size();
    } else {
      // The geodist() function has been called with a positive max_dist setting, and it returned 0.0 for all vertices it
      // did not visit in that case. That is unfortunate, so we fix the returned distance values here.
      // If we do not fix this, the _compute_geodesic_circle_stats() function will produce wrong results based on the distances.
      for(size_t j=0; j<v_geodist.size(); j++) {
        if(j != (size_t)qv) { // If j==qv, the distance is from the query vertex to itself, and it is *really* zero.
          if(v_geodist[j] <= 0.000000001) {
            v_geodist[j] = max_possible_float;
          }
        }
      }
    }

    std::vector<double> sample_at_radii = linspace<double>(r_cycle-10.0, r_cycle+10.0, sampling);
    std::vector<std::vector<double>> circle_stats = _compute_geodesic_circle_stats(m, v_geodist, sample_at_radii);
    std::vector<double> circle_areas = circle_stats[0];
    std::vector<double> circle_perimeters = circle_stats[1];

    assert(sample_at_radii.size() == circle_areas.size());
    assert(sample_at_radii.size() == circle_perimeters.size());

    std::vector<double> x = linspace<double>(1.0, sampling, numsteps_for_stepsize(1.0, sampling, 1.0)); // spline x values
    std::vector<double> xx = linspace<double>(1.0, sampling, numsteps_for_stepsize(1.0, sampling, 0.1));  // where to sample
    int num_samples = xx.size();

    assert(x.size() == circle_areas.size()); // If this fails, there is a bug in the numsteps_for_stepsize() function.

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


