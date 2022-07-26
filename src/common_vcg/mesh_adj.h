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


/// Write mesh edge adjacency data to a JSON file.
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


/// Write mesh edge adjacency data to a CSV file.
///
/// neigh_write_size: the number of neihbors to write for each vertex (number of neighbor columns). If shorther
///             than actual number of neighbors, the list will be truncated. If longer than the real available
///             number of neighbors, the behavior depends on the setting of allow_nan. Set to 0 for 'use the min of all neighborhood sizes'.
/// allow_nan: whether to allow nan values in the output file. If neigh_write_size is larger than actual neighborhood and
///            this setting is true, the missing values will be written as NANs. Otherwise, an error will be raised.
/// Dont roll your own CSV, they told us.
std::string edge_neigh_to_csv(std::vector<std::vector<int>> neigh, size_t neigh_write_size = 0, bool allow_nan = false, bool header=true) {

    // Get min size over all neighborhoods.
    size_t min_neighbor_count = (size_t)-1;  // Set to max possible value.
    for(size_t i=0; i < neigh.size(); i++) {
      if(neigh[i].size() < min_neighbor_count) {
        min_neighbor_count = neigh[i].size();
      }
    }
    if(neigh_write_size == 0) {
      neigh_write_size = min_neighbor_count;
    }

    // Pre-check is allow_nan is false, so we do not start writing something that will not be finished.
    std::vector<int> failed_neighborhoods;
    for(size_t i=0; i < neigh.size(); i++) {
      if(neigh[i].size() < neigh_write_size) {
        failed_neighborhoods.push_back(i);
      }
    }
    if(! allow_nan) {
      if(failed_neighborhoods.size() >= 1) {
        throw std::runtime_error("Failed to generate mesh edge neighborhood CSV representation:'" + std::to_string(failed_neighborhoods.size()) + " neighborhoods are smaller than neigh_write_size "  + std::to_string(neigh_write_size) + ", and allow_nan is false.\n");
      }
    }

    std::stringstream is;
    if(header) {  // Write header line like: 'source n0 n1 n2 ...'
      is << "source ";
      for(size_t i=0; i < neigh_write_size; i++) {
        is << "n" << i;
        if(i < neigh_write_size - 1) {
          is << " ";
        }
      }
      is << "\n";
    }

    // Now for the data.
    for(size_t i=0; i < neigh.size(); i++) {

      // We write one line per neighborhood (source vertex)
      is << i;  // Write the source vertex.
      for(size_t j=0; j < neigh_write_size; j++) {
          if(j < neigh[i].size()) {  // Write the neighborhood data.
            is << " " << neigh[i][j];
          } else {  // Fill with NA values. We know that allow_nan is true, otherwise we had thrown an error earlier.
            is << " NA";
          }
      }
      is << "\n";  // End CSV line.
    }

    return is.str();
}


/// Write a string to a text file.
///
/// TODO: Currently this only print to stderr in case of errors, we should most likely throw an exception instead.
void strtofile(std::string outstring, std::string filename) {
    std::ofstream outs(filename);
    if(outs.is_open()) {
        outs << outstring;
        outs.close();
    } else {
        std::cerr << "Failed to open output file '" << filename << "'.\n";
    }
}
