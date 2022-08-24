#pragma once

#include "libfs.h"

#include "typedef_vcg.h"


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <cassert>



/// @brief Struct modeling a spatial neighborhood with coordinates, and geodesic or Euclidean distances of neighbors to central vertex.
///
/// @details The coordinates of the neighbors should be
/// centered on the central/source vertex (i.e., it should be at (0,0,0)) but
/// NOT scaled (e.g., to range 0..1).
/// Note that the fact that the central vertex is the origin means that we do
/// not need to store its coordinates. If you need that (and do not center), you
/// will have to add the vertex itself to its neighborhood.
struct Neighborhood {
  /// Constructor to initialize everything.
  Neighborhood(size_t index, std::vector<std::vector<float>> coords, std::vector<float> distances, std::vector<std::vector<float>> normals) : index(index), coords(coords), distances(distances), normals(normals) {}
  /// Constructor to initialize everything but normals.
  Neighborhood(size_t index, std::vector<std::vector<float>> coords, std::vector<float> distances) : index(index), coords(coords), distances(distances), normals(std::vector<std::vector<float>>(coords.size(), std::vector<float>(3, 0.0))) {}
  /// Constructor to initialize central vertex index and all neighbor coords.
  Neighborhood(size_t index, std::vector<std::vector<float>> coords) : index(index), coords(coords), distances(std::vector<float>(coords.size(), 0.0)), normals(std::vector<std::vector<float>>(coords.size(), std::vector<float>(3, 0.0))) {}
  /// Default constructor.
  Neighborhood() : index(0), coords(std::vector<std::vector<float>>(0)), distances(std::vector<float>(0)), normals(std::vector<std::vector<float>>(0)) {}

  size_t index; ///< The index of the central/source vertex.
  std::vector<std::vector<float>> coords; ///< 2D array of neighborhood vertex coordinates, for n vertices this will have dimensions (n, 3).
  std::vector<float> distances;  ///< distances from the central vertex for all n neighbors -- these can be Euclidean (easily derivable from coords) or Geodesic.
  std::vector<std::vector<float>> normals; ///< vertex normals.
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

/// Dont roll your own JSON, they told us.
/// @brief Get JSON representation on mesh neighborhoods.
/// TODO: This could become a static method of Neighborhood
std::string neighborhoods_to_json(std::vector<Neighborhood> neigh) {
    std::stringstream is;
    is << "{\n";
    is << "  \"neighborhoods\": {\n";
    Neighborhood nh;
    for(size_t i=0; i < neigh.size(); i++) {
      nh = neigh[i];
      is << "  \"" << nh.index << "\": {\n";  // TODO: add coords, distances, normals here.
      is << "    \"coords\": [\n";
      is << "      ],\n";
      is << "    \"distances\": [\n";
      is << "      ],\n";
      is << "    \"normals\": [\n";
      is << "      ],\n";
      is << "    }\n";
    }
    is << "  }\n";
    is << "}\n";
    is << "not implemented yet\n";
    std::cerr << "neighborhoods_to_json: not implemented yet.\n";
    return is.str();
}

/// @brief Write Neighborhoods vector to CSV string representation.
std::string neighborhoods_to_csv(std::vector<Neighborhood> neigh) {
    std::stringstream is;
    is << "not implemented yet\n";
    std::cerr << "neighborhoods_to_csv: not implemented yet.\n";
    return is.str();
}