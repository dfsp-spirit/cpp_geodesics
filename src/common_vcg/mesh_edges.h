#pragma once

#include "typedef_vcg.h"

typedef UpdateTopology<MyMesh>::PEdge SimpleEdge;

/// @brief Compute all edge lengths of the VCG mesh.
/// @param m VCGLIB mesh
/// @return vector of edge lengths for all mesh edges
std::vector<double> mesh_edge_lengths(MyMesh& m) {
    m.vert.EnableVFAdjacency();
    m.face.EnableFFAdjacency();
    m.face.EnableVFAdjacency();
    std::vector<SimpleEdge> edges;
    std::vector<SimpleEdge>::iterator ei;
    tri::UpdateTopology<MyMesh>::FaceFace(m);
    tri::UpdateTopology<MyMesh>::FillUniqueEdgeVector(m, edges, true);
    size_t num_edges = edges.size();
    std::vector<double> edgelength(num_edges);
    Point3f tmp;
    VertexPointer vp , vp1;
    for (std::vector<SimpleEdge>::size_type i = 0; i < num_edges; i++) {
      vp = edges[i].v[0];
      vp1 = edges[i].v[1];
      tmp = vp->P() - vp1->P();
      edgelength[i] = sqrt(tmp.dot(tmp));
    }
    return edgelength;
}
