// The main for the geo program. This uses third_party/geodesic algorithms.
//
// This is based on the 2nd example from the github.com/mojocorp/geodesic/ repo.

#include <iostream>
#include <fstream>
#include <string>

#include "libfs.h"
#include <geodesic_algorithm_dijkstra.h>
#include <geodesic_algorithm_subdivision.h>
#include <geodesic_algorithm_exact.h>


// Some utility functions.
namespace geodesic {

    inline double length(const std::vector<SurfacePoint>& path) {
        double length = 0;
        if (!path.empty()) {
            for (unsigned i = 0; i < path.size() - 1; ++i) {
                length += path[i].distance(path[i + 1]);
            }
        }
        return length;
    }

    inline void print_path(const std::vector<SurfacePoint>& path) {
        std::cout << "Path from " << std::to_string(source) << " to " << std::to_string(target) << "(" << std::to_string(path.size() - 1) << "):";
        if (!path.empty()) {
            for (unsigned i = 0; i < path.size() - 1; ++i) {
                std::cout << " " << std::to_string(path[i]) << "\n";
            }
        }                
    }

}


int main(int argc, char** argv) {
    std::vector<double> points;
    std::vector<unsigned> faces;    

    std::string mesh_file;
    size_t source = 0;
    size_t target = 100;

    
    if(argc < 2 || argc > 4) {
        std::cout << "===" << argv[0] << " -- Compute geodesic path and distance on a mesh. ===\n";
        std::cout << "Usage: " << argv[0] << " [<mesh> [<source> [<target>]]]\n";
        std::cout << "  <mesh>   : str, path to the input mesh file.\n";
        std::cout << "  <source> : int >= 0, the source vertex (0-based index).\n";
        std::cout << "  <target> : int >= 0, the target vertex (0-based index).\n";
        exit(1);
    }
    if(argc >= 2) {
        mesh_file = argv[1];
    }
    if(argc >= 3) {
        source = argv[2];
    }
    if(argc >= 4) {
        target = argv[3];
    }

    std::cout << " Reading mesh file '" + mesh_file + "'...\n";    
    fs::Mesh surface;
    fs::read_mesh(&surface, surf_file);

    geodesic::Mesh mesh;
    mesh.initialize_mesh_data(surface.vertices, surface.faces); // create 'geodesic' lib mesh data structure

    geodesic::GeodesicAlgorithmExact exact_algorithm(&mesh); // exact algorithm
    geodesic::GeodesicAlgorithmDijkstra dijkstra_algorithm(&mesh); // simplest approximate algorithm: path only allowed on the edges of the mesh
    unsigned const subdivision_level = 3; // three additional vertices per every edge in subdivision algorithm
    geodesic::GeodesicAlgorithmSubdivision subdivision_algorithm(&mesh, subdivision_level); // with subdivision_level=0 this algorithm becomes Dijkstra, with subdivision_level->infinity it becomes exact

    std::vector<geodesic::GeodesicAlgorithmBase*> all_algorithms; // for simplicity, store all possible geodesic algorithms here
    all_algorithms.push_back(&dijkstra_algorithm);
    all_algorithms.push_back(&subdivision_algorithm);
    all_algorithms.push_back(&exact_algorithm);

    std::vector<geodesic::SurfacePoint> sources;
    sources.push_back(geodesic::SurfacePoint(&mesh.vertices()[source])); // one source is located at vertex zero

    std::vector<geodesic::SurfacePoint> targets; // same thing with targets
    targets.push_back(geodesic::SurfacePoint(&mesh.vertices()[target]));
 
    for (unsigned index = 0; index < all_algorithms.size(); ++index) {
        geodesic::GeodesicAlgorithmBase* algorithm = all_algorithms[index]; // all algorithms are derived from GeodesicAlgorithmBase
        std::cout << std::endl << "results for algorithm " << algorithm->name() << std::endl;

        algorithm->propagate(sources); // cover the whole mesh

        //-------------first task: compute the pathes to the targets -----
        std::vector<geodesic::SurfacePoint> path;
        for (size_t i = 0; i < targets.size(); ++i) {
            algorithm->trace_back(targets[i], path);
            print_path(path);
        }

    }
    return 0;
}
