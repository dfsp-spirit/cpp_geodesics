#pragma once

#include "libfs.h"

#include "typedef_vcg.h"
#include "mesh_normals.h"
#include "mesh_coords.h"
#include "write_data.h"


#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <cassert>
#include <limits>


#ifndef APPTAG
#define APPTAG "[cpp_geod] "
#endif



/// @brief Struct modeling a spatial neighborhood with coordinates, and geodesic or Euclidean distances of neighbors to central vertex.
///
/// @details The coordinates of the neighbors should be
/// centered on the central/source vertex (i.e., it should be at (0,0,0)) but
/// NOT scaled (e.g., to range 0..1).
/// Note that the fact that the central vertex is the origin means that we do
/// not need to store its coordinates. If you need that (and do not center), you
/// will have to add the vertex itself to its neighborhood.
class Neighborhood {
  public:
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

  /// @brief Get number of entries (neighbor vertices) in this neighborhood.
  /// @return number of entries (neighbor vertices) in this neighborhood.
  size_t size() {
    return this->distances.size();
  }

  /// @brief Turn neighborhood into a vector representing a CSV or whatever row.
  /// @param neigh_write_size the number of vertices to consider in the neighborhood. If more than its size, will be filled with NAN. If less, the rest will be ignored.
  /// @param pvd float, per-vertex descriptor value for this vertex. See `use_pvd` below if you do not want/have this.
  /// @param use_pvd bool, whether to write the value 'pvd' to the ouput (if not, resulting returned vector will be shorter).
  /// @param normals whether to write the normals. Can set to false if they are empty or filled with junk because you did not compute them.
  /// @return float vector representation of neighborhood, handy for CSV or vvbin export
  std::vector<float> to_row(const size_t neigh_write_size, const float pvd=0.0, const bool use_pvd=false, const bool normals=true) {
    size_t row_length = 1 + ((3 + 1) * neigh_write_size); // the source index, plus for each neighbor: the 3 coords (x,y,z), the distance
    if(normals) {
      row_length += 3 * neigh_write_size; // for each neighbor: the normals
    }
    if(use_pvd) {
      row_length += 1; // the label (central vertex descriptor value)
    }
    std::vector<float> row = std::vector<float>(row_length, 0.0);

    size_t num_written = 0;
    // write source index
    row[num_written] = (float)this->index; num_written++;

    // Now for the data: coordinates, distances, and normals

    for(size_t j=0; j < neigh_write_size; j++) {  // Write the neighbor vertex coords.
      if(j < this->size()) {
        row[num_written] = this->coords[j][0]; num_written++;
        row[num_written] = this->coords[j][1]; num_written++;
        row[num_written] = this->coords[j][2]; num_written++;
      } else {  // Fill with NA values. We know that allow_nan is true, otherwise we had thrown an error earlier.
        row[num_written] = NAN; num_written++;
        row[num_written] = NAN; num_written++;
        row[num_written] = NAN; num_written++;
      }
    }

    for(size_t j=0; j < neigh_write_size; j++) {  // Write the neighbor distances.
      if(j < this->size()) {
        row[num_written] = this->distances[j]; num_written++;
      } else {  // Fill with NA values. We know that allow_nan is true, otherwise we had thrown an error earlier.
        row[num_written] = NAN; num_written++;
      }
    }

    if(normals) {
      for(size_t j=0; j < neigh_write_size; j++) {  // Write the neighbor normals.
        if(j < this->size()) {
          row[num_written] = this->normals[j][0]; num_written++;
          row[num_written] = this->normals[j][1]; num_written++;
          row[num_written] = this->normals[j][2]; num_written++;
        } else {  // Fill with NA values. We know that allow_nan is true, otherwise we had thrown an error earlier.
          row[num_written] = NAN; num_written++;
          row[num_written] = NAN; num_written++;
          row[num_written] = NAN; num_written++;
        }
      }
    }
    if(use_pvd) {
      row[num_written] = pvd; num_written++;
    }
    assert(num_written == row_length);
    return(row);
  }
};

/// @brief Compute vertex neighborhoods: for a source vertex, compute centered coordinates of all given neighbors.
/// @details The distances in the return value are geodesic distances.
/// @param geod_neighbors: (n, m) 2D vector of `GeodNeighbor`, typically the neighborhoods (each consisting of `m` neighbors) for all `n` vertices of some mesh. Neighbors are encoded as vertex indices in the GeodNeighbor struct.
/// @param mesh: the mesh, used to get the vertex coordinates from the vertex indices in geod_neighbors.
/// @return vector of `n` Neighborhood instances
std::vector<Neighborhood> neighborhoods_from_geod_neighbors(const std::vector<std::vector<GeodNeighbor> > geod_neighbors, MyMesh &mesh) {
  size_t num_neighborhoods = geod_neighbors.size();
  std::cout << std::string(APPTAG) << "Computing neighborhoods for " << num_neighborhoods << " vertices and their geodesic neighbors." << "\n";
  std::vector<Neighborhood> neighborhoods;
  size_t neigh_size;
  std::vector<std::vector<float>> neigh_coords;
  std::vector<std::vector<float>> neigh_normals;
  std::vector<float> neigh_distances;
  std::vector<float> source_vert_coords;
  std::vector<int> neigh_indices;

  std::vector<std::vector<float>> m_vnormals = mesh_vnormals(mesh);
  std::vector<std::vector<float>> m_vcoords = mesh_vertex_coords(mesh);

  size_t central_vert_mesh_idx;
  size_t neigh_mesh_idx;
  for(size_t i = 0; i < num_neighborhoods; i++) {
    central_vert_mesh_idx = i;
    neigh_size = geod_neighbors[i].size();
    neigh_indices = std::vector<int>(neigh_size);
    neigh_distances = std::vector<float>(neigh_size);
    neigh_coords = std::vector<std::vector<float> >(neigh_size, std::vector<float> (3, 0.0));
    neigh_normals = std::vector<std::vector<float> >(neigh_size, std::vector<float> (3, 0.0));
    for(size_t j = 0; j < neigh_size; j++) {
      //std::cout << ">>> Handling neighborhood #" << i << " of " << num_neighborhoods << " with size " << neigh_size << "\n";
      neigh_mesh_idx = geod_neighbors[i][j].index; // absolute index (in full mesh vertex vector)
      neigh_indices[j] = neigh_mesh_idx;
      neigh_distances[j] = geod_neighbors[i][j].distance;  // This is the geodesic distance in this case!
      neigh_coords[j] = std::vector<float> {m_vcoords[neigh_mesh_idx][0], m_vcoords[neigh_mesh_idx][1], m_vcoords[neigh_mesh_idx][2]};
      source_vert_coords = std::vector<float> {m_vcoords[central_vert_mesh_idx][0], m_vcoords[central_vert_mesh_idx][1], m_vcoords[central_vert_mesh_idx][2]};
      neigh_normals[j] = std::vector<float> {m_vnormals[neigh_mesh_idx][0], m_vnormals[neigh_mesh_idx][1], m_vnormals[neigh_mesh_idx][2]};
      // Center the coords around source vertex (make it the origin):
      for(size_t k = 0; k < 3; k++) {
        neigh_coords[j][k] -= source_vert_coords[k];
      }
    }
    neighborhoods.push_back(Neighborhood(i, neigh_coords, neigh_distances, neigh_normals));
  }
  return neighborhoods;
}

/// @brief Computes neighborhoods where the distance is the geodesic distance.
/// @param edge_neighbors compute edge neighbors, see
/// @param mesh VCGLIB mesh instance
/// @param keep_verts vector with same length as edge_neighbors, whether to keep a certain vertex (neighborhood around this vertex). If left at default or empty vector is passed instead, all vertices will be kept (no filtering happens). Note that vertices ignored as centers of neighborhoods may still show up as part of a neighborhood of another source vertex.
/// @details The distances in the return value are Euclidean distances.
std::vector<Neighborhood> neighborhoods_from_edge_neighbors(const std::vector<std::vector<int> > edge_neighbors, MyMesh &mesh, std::vector<bool> keep_verts = std::vector<bool>()) {

  size_t num_neighborhoods = edge_neighbors.size();

  if(keep_verts.empty()) {
    keep_verts = std::vector<bool>(num_neighborhoods, true);
  }
  assert(keep_verts.size() == num_neighborhoods); // Can fail if keep_verts was passed non-empty.

  std::vector<Neighborhood> neighborhoods;
  size_t neigh_size, neigh_mesh_idx;
  std::vector<std::vector<float>> neigh_coords;
  std::vector<std::vector<float>> neigh_normals;
  std::vector<float> neigh_distances;
  std::vector<float> source_vert_coords;
  std::vector<int> neigh_indices;

  std::vector<std::vector<float>> m_vnormals = mesh_vnormals(mesh);
  std::vector<std::vector<float>> m_vcoords = mesh_vertex_coords(mesh);

  size_t central_vert_mesh_idx;
  for(size_t i = 0; i < num_neighborhoods; i++) {
    if(! keep_verts[i]) {
      continue;
    }
    central_vert_mesh_idx = i;
    neigh_size = edge_neighbors[i].size();
    //std::cout << ">>> Handling neighborhood #" << i << " of " << num_neighborhoods << " with size " << neigh_size << "\n";
    neigh_indices = std::vector<int>(neigh_size);
    neigh_distances = std::vector<float>(neigh_size);
    neigh_coords = std::vector<std::vector<float> >(neigh_size, std::vector<float> (3, 0.0));
    neigh_normals = std::vector<std::vector<float> >(neigh_size, std::vector<float> (3, 0.0));
    for(size_t j = 0; j < neigh_size; j++) {
      //std::cout << ">>>   Handling neighborhood #" << i << " neighbor# " << j << " of " << neigh_size << ".\n";
      neigh_mesh_idx = edge_neighbors[i][j];
      //std::cout << ">>>   abs mesh index of this neighbor is " << neigh_mesh_idx << ", mesh has " << mesh.vn << " vertices.\n";
      neigh_indices[j] = neigh_mesh_idx;
      neigh_coords[j] = std::vector<float> {m_vcoords[neigh_mesh_idx][0], m_vcoords[neigh_mesh_idx][1], m_vcoords[neigh_mesh_idx][2]};
      neigh_normals[j] = std::vector<float> {m_vnormals[neigh_mesh_idx][0], m_vnormals[neigh_mesh_idx][1], m_vnormals[neigh_mesh_idx][2]};
      source_vert_coords = std::vector<float> {m_vcoords[central_vert_mesh_idx][0], m_vcoords[central_vert_mesh_idx][1], m_vcoords[central_vert_mesh_idx][2]};
      neigh_distances[j] = dist_euclid(neigh_coords[j], source_vert_coords); // This is the Euclidean distance in this case!
      // Center the coords around source vertex (make it the origin):
      for(size_t k = 0; k < 3; k++) {
        neigh_coords[j][k] -= source_vert_coords[k];
      }
    }
    neighborhoods.push_back(Neighborhood(i, neigh_coords, neigh_distances, neigh_normals));
  }
  return neighborhoods;
}

/// Dont roll your own JSON, they told us.
/// @brief Get JSON representation on mesh neighborhoods.
/// TODO: This could become a static method of Neighborhood
std::string neighborhoods_to_json(std::vector<Neighborhood> neigh) {
  // NOT READY YET
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
/// @param neigh_write_size: the number of neihbors to write for each vertex (number of neighbor columns). If shorther
///             than actual number of neighbors, the list will be truncated. If longer than the real available
///             number of neighbors, the behavior depends on the setting of allow_nan. Set to 0 for 'use the min of all neighborhood sizes'.
/// @param allow_nan: whether to allow nan values in the output file. If neigh_write_size is larger than actual neighborhood and
///            this setting is true, the missing values will be written as NANs. Otherwise, an error will be raised.
/// @param header: whether to write a header line
/// @param normals whether to write vertex normals
/// @return CSV string representation of edge neighborhoods
std::string neighborhoods_to_csv(std::vector<Neighborhood> neigh, size_t neigh_write_size = 0, const bool allow_nan = false, const bool header=true, const bool normals = true, const std::string& input_pvd_file = "") {

  // Read per-vertex data (thickness, pial_lGI, or whatever), if a filename for it was given.
  std::vector<float> pvd;
  if(! input_pvd_file.empty()) {
    pvd = fs::read_curv_data(input_pvd_file);
  }

  // Get min size over all neighborhoods.
  size_t min_neighbor_count = (size_t)-1;  // Set to max possible value.
  size_t max_neighbor_count = 0; // FYI, only used in the log output.
  for(size_t i=0; i < neigh.size(); i++) {
    if(neigh[i].size() < min_neighbor_count) {
      min_neighbor_count = neigh[i].size();
    }
    if(neigh[i].size() > max_neighbor_count) {
      max_neighbor_count = neigh[i].size();
    }
  }

  if(neigh_write_size == 0) {
      neigh_write_size = min_neighbor_count;
      debug_print(CPP_GEOD_DEBUG_LVL_IMPORTANT, "Using auto-determined neighborhood size " + std::to_string(neigh_write_size) + " during Neighborhood CSV export.");
  }

  debug_print(CPP_GEOD_DEBUG_LVL_INFO, "Exporting " + std::to_string(neigh.size()) + " neighborhoods, with " + std::to_string(neigh_write_size) + " entries per neighborhood. Min neighborhood size = " + std::to_string(min_neighbor_count) + ", max = " + std::to_string(max_neighbor_count) + ".");

  // Pre-check if allow_nan is false, so we do not start writing something that will not be finished.
  std::vector<int> failed_neighborhoods; // These will only 'fail' if NAN values are not allowed.
  for(size_t i=0; i < neigh.size(); i++) {
    if(neigh[i].size() < neigh_write_size) {
      failed_neighborhoods.push_back(i);
    }
  }
  if(! allow_nan) {
    if(failed_neighborhoods.size() >= 1) {
      throw std::runtime_error("Failed to generate mesh neighborhood CSV representation:'" + std::to_string(failed_neighborhoods.size()) + " neighborhoods are smaller than neigh_write_size "  + std::to_string(neigh_write_size) + ", and allow_nan is false.\n");
    }
  } else {
    std::cout << std::string(APPTAG) << "There are " << failed_neighborhoods.size() << " neighborhoods smaller than neigh_write_size " << neigh_write_size << ", will pad with 'NA' values.";
  }

  // Write header for coordinates, distances, and normals
  std::stringstream is;
  if(header) {  // Write header line like: 'source n0cx n0cy n0cz ... n0dist ... n0nx n0ny n0nz', where 'cx' is for coord x, and 'nx' is for normal x.
    is << "source ";
    for(size_t i=0; i < neigh_write_size; i++) { // header for the neighbor coords, 3 per vertex
      is << "n" << i << "cx" << " " << "n" << i << "cy" << " " << "n" << i << "cz";
      if(i < neigh_write_size - 1) {
        is << " ";
      }
    }
    is << " ";
    for(size_t i=0; i < neigh_write_size; i++) { // header for the neighbor distances, 1 per vertex
      is << "n" << i << "dist";
      if(i < neigh_write_size - 1) {
        is << " ";
      }
    }
    if(normals) {
      is << " ";
      for(size_t i=0; i < neigh_write_size; i++) { // header for the neighbor vertex normals, 3 per vertex
        is << "n" << i << "nx" << " " << "n" << i << "ny" << " " << "n" << i << "nz";
        if(i < neigh_write_size - 1) {
          is << " ";
        }
      }
    }
    if(! input_pvd_file.empty()) {  // Write header for the label, i.e., the per-vertex descritptor data for this vertex.
      is << " label";
    }
    is << "\n"; // terminate header line.
  }

  // Report on neigh dists for user.
  bool do_report = true;
  if(do_report) {
    float min_neigh_dist = std::numeric_limits<float>::max();
    float max_neigh_dist = 0.0;
    float dist_sum = 0.0;
    size_t num_dists_considered = 0;
    for(size_t i=0; i < neigh.size(); i++) {
      for(size_t j=0; j < neigh_write_size; j++) {  // Write the neighbor distances.
        if(j < neigh[i].size()) {
          num_dists_considered++;
          dist_sum += neigh[i].distances[j];
          if(neigh[i].distances[j] < min_neigh_dist) {
            min_neigh_dist = neigh[i].distances[j];
          }
          if(neigh[i].distances[j] > max_neigh_dist) {
            max_neigh_dist = neigh[i].distances[j];
          }
        }
      }
    }
    float mean_neigh_dist = dist_sum / (float)num_dists_considered;
    std::cout << std::string(APPTAG) << "For exported neighborhoods (" << neigh_write_size << " entries max), the minimal distance is " << min_neigh_dist << ", mean is " << mean_neigh_dist << ", and max is " << max_neigh_dist << ".\n";
  }

  // Now for the data: coordinates, distances, and normals
  for(size_t i=0; i < neigh.size(); i++) {

    // We write one line per neighborhood (source vertex)
    is << neigh[i].index;  // Write the source vertex.
    for(size_t j=0; j < neigh_write_size; j++) {  // Write the neighbor vertex coords.
      if(j < neigh[i].size()) {
        is << " " << neigh[i].coords[j][0] << " " << neigh[i].coords[j][1] << " " << neigh[i].coords[j][2];
      } else {  // Fill with NA values. We know that allow_nan is true, otherwise we had thrown an error earlier.
        is << " NA NA NA";
      }
    }
    for(size_t j=0; j < neigh_write_size; j++) {  // Write the neighbor distances.
      if(j < neigh[i].size()) {
        is << " " << neigh[i].distances[j];
      } else {  // Fill with NA values. We know that allow_nan is true, otherwise we had thrown an error earlier.
        is << " NA";
      }
    }
    if(normals) {
      for(size_t j=0; j < neigh_write_size; j++) {  // Write the neighbor normals.
        if(j < neigh[i].size()) {
          is << " " << neigh[i].normals[j][0] << " " << neigh[i].normals[j][1] << " " << neigh[i].normals[j][2];
        } else {  // Fill with NA values. We know that allow_nan is true, otherwise we had thrown an error earlier.
          is << " NA NA NA";
        }
      }
    }
    if(! input_pvd_file.empty()) {
      is << " " << pvd[neigh[i].index];  // Write descriptor value.
    }
    is << "\n";  // End CSV line.
  }
  return is.str();
}


std::vector<std::vector<float>> neighborhoods_to_vvbin(std::vector<Neighborhood> neigh, size_t neigh_write_size = 0, const bool allow_nan = false, const bool normals = true, const std::string& input_pvd_file = "") {
  // Read per-vertex data (thickness, pial_lGI, or whatever), if a filename for it was given.
  std::vector<float> pvd;
  if(! input_pvd_file.empty()) {
    pvd = fs::read_curv_data(input_pvd_file);
  } else {
    pvd = std::vector<float>(neigh.size(), 0.0); // fill with zeros
  }

  // Get min size over all neighborhoods.
  size_t min_neighbor_count = (size_t)-1;  // Set to max possible value.
  size_t max_neighbor_count = 0; // FYI, only used in the log output.
  for(size_t i=0; i < neigh.size(); i++) {
    if(neigh[i].size() < min_neighbor_count) {
      min_neighbor_count = neigh[i].size();
    }
    if(neigh[i].size() > max_neighbor_count) {
      max_neighbor_count = neigh[i].size();
    }
  }

  if(neigh_write_size == 0) {
      neigh_write_size = min_neighbor_count;
      debug_print(CPP_GEOD_DEBUG_LVL_INFO, "Using auto-determined neighborhood size " + std::to_string(neigh_write_size) + " during Neighborhood vvbin export.\n");
  }

  debug_print(CPP_GEOD_DEBUG_LVL_INFO, "Exporting " + std::to_string(neigh.size()) + " neighborhoods, with " + std::to_string(neigh_write_size) + " entries per neighborhood. Min neighborhood size = " + std::to_string(min_neighbor_count) + ", max = " + std::to_string(max_neighbor_count) + ".");

  // Pre-check if allow_nan is false, so we do not start writing something that will not be finished.
  std::vector<int> failed_neighborhoods; // These will only 'fail' if NAN values are not allowed.
  for(size_t i=0; i < neigh.size(); i++) {
    if(neigh[i].size() < neigh_write_size) {
      failed_neighborhoods.push_back(i);
    }
  }
  if(! allow_nan) {
    if(failed_neighborhoods.size() >= 1) {
      throw std::runtime_error("Failed to generate mesh neighborhood vvbin representation:'" + std::to_string(failed_neighborhoods.size()) + " neighborhoods are smaller than neigh_write_size "  + std::to_string(neigh_write_size) + ", and allow_nan is false.\n");
    }
  } else {
    std::cout << std::string(APPTAG) << "There are " << failed_neighborhoods.size() << " neighborhoods smaller than neigh_write_size " << neigh_write_size << ", will pad with 'NA' values.";
  }

  // Now for the actual data: coordinates, distances, and normals
  size_t row_length = 1 + ((3 + 1) * neigh_write_size); // the source index, plus for each neighbor: the 3 coords (x,y,z), the distance
  if(normals) {
    row_length += 3 * neigh_write_size; // for each neighbor: the normals
  }
  if(! input_pvd_file.empty()) {
    row_length += 1; // the label (central vertex descriptor value)
  }

  std::vector<std::vector<float>> neigh_mat = std::vector<std::vector<float> >(neigh.size(), std::vector<float> (row_length));
  for(size_t i=0; i < neigh.size(); i++) {
    neigh_mat[i] = neigh[i].to_row(neigh_write_size, pvd[neigh[i].index], (! input_pvd_file.empty()), normals);
  }
  return neigh_mat;
}