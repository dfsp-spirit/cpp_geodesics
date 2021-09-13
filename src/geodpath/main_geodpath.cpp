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

    /// Get geodesic path length in edge units (double).
    inline double path_length(const std::vector<SurfacePoint>& path) {
        double length = 0;
        if (!path.empty()) {
            for (unsigned i = 0; i < path.size() - 1; ++i) {
                length += path[i].distance(path[i + 1]);
            }
        }
        return length;
    }

    /// Get string representation of path xzy coords.
    inline std::string path_rep(const std::vector<SurfacePoint>& path) {        
        std::string path_rep = "";
        if (!path.empty()) {
            for (unsigned i = 0; i < path.size() - 1; ++i) {
                path_rep += "(";
                path_rep += (std::to_string(path[i].x()) + ", ");
                path_rep += (std::to_string(path[i].y()) + ", ");
                path_rep += (std::to_string(path[i].z()) + ")");
                if(i < path.size() - 2) {
                    path_rep += ", ";
                }
            }            
        }
        return(path_rep);            
    }

}


int main(int argc, char** argv) {
    std::vector<double> points;
    std::vector<unsigned> faces;    

    std::string mesh_file;
    size_t source = 0;
    size_t target = 100;
    size_t algo = 2;
    size_t subdivision_level = 3; // Number of additional vertices per every edge in subdivision algorithm.

    
    if(argc < 2 || argc > 6) {
        std::cout << "===" << argv[0] << " -- Compute geodesic path and distance on a mesh. ===\n";
        std::cout << "Usage: " << argv[0] << " [<mesh> [<source> [<target>]]]\n";
        std::cout << "  <mesh>   : str, path to the input mesh file.\n";
        std::cout << "  <source> : int >= 0, the source vertex (0-based index).\n";
        std::cout << "  <target> : int >= 0, the target vertex (0-based index).\n";
        std::cout << "  <algo>   : int >= 0, alogorithm to run. 0=all, 1=exact, 2=dijksta, 3=subdivision dijksta. Default 2.\n";
        std::cout << "  <subd>   : int >= 1, number of edge subdivisions for algo 3. Defaults to 3.\n";
        exit(1);
    }
    if(argc >= 2) {
        mesh_file = argv[1];
    }
    if(argc >= 3) {        
        std::istringstream iss( argv[2] );
        if(!(iss >> source)) {
            throw std::runtime_error("Could not convert argument 'source' to integer.\n");
        }        
    }            
    if(argc >= 4) {
        std::istringstream iss( argv[3] );
        if(!(iss >> target)) {
            throw std::runtime_error("Could not convert argument 'target' to integer.\n");
        }
    }
    if(argc >= 5) {
        std::istringstream iss( argv[4] );
        if(!(iss >> algo)) {
            throw std::runtime_error("Could not convert argument 'algo' to integer.\n");
        }
        if(algo >= 4) {
            throw std::runtime_error("Argument 'algo' out of range.\n");
        }
        if(argc >= 6) {
            if(algo == 3 || algo == 0) {
                std::istringstream iss( argv[5] );
                if(!(iss >> subdivision_level)) {
                    throw std::runtime_error("Could not convert argument 'subdivision_level' to integer.\n");
                }
                if(subdivision_level < 1) {
                    throw std::runtime_error("Argument 'subdivision_level' out of range.\n");
                }
                std::cout << "Using " << std::to_string(subdivision_level) << " subdivisions for algorithm 'subdivision dijksta'.\n";
            } else {
                throw std::runtime_error("Argument 'subd' invalid unless algo is 0 or 3.");
            }
        }        
    }

    std::cout << "Running algorithm " + std::to_string(algo) + " on mesh file '" + mesh_file + "'...\n";    
    fs::Mesh surface;
    fs::read_mesh(&surface, mesh_file);

    if(source >= surface.num_vertices()) {
        throw std::runtime_error("Source vertex index " + std::to_string(source) + " invalid for mesh with " + std::to_string(surface.num_vertices()) + " vertices (and 0-based indices).\n");
    }
    if(target >= surface.num_vertices()) {
        throw std::runtime_error("Target vertex index " + std::to_string(target) + " invalid for mesh with " + std::to_string(surface.num_vertices()) + " vertices (and 0-based indices).\n");
    }

    geodesic::Mesh mesh;
    mesh.initialize_mesh_data(surface.vertices, surface.faces, true);

    geodesic::GeodesicAlgorithmExact exact_algorithm(&mesh); // exact algorithm
    geodesic::GeodesicAlgorithmDijkstra dijkstra_algorithm(&mesh); // simplest approximate algorithm: path only allowed on the edges of the mesh
    geodesic::GeodesicAlgorithmSubdivision subdivision_algorithm(&mesh, subdivision_level); // with subdivision_level=0 this algorithm becomes Dijkstra, with subdivision_level->infinity it becomes exact

    std::vector<geodesic::GeodesicAlgorithmBase*> all_algorithms;
    all_algorithms.push_back(&exact_algorithm);
    all_algorithms.push_back(&dijkstra_algorithm);
    all_algorithms.push_back(&subdivision_algorithm);

    std::vector<geodesic::SurfacePoint> sources;
    sources.push_back(geodesic::SurfacePoint(&mesh.vertices()[source]));

    std::vector<geodesic::SurfacePoint> targets;
    targets.push_back(geodesic::SurfacePoint(&mesh.vertices()[target]));
 
    for (unsigned index = 0; index < all_algorithms.size(); ++index) {
        if((index + 1) == algo || algo == 0) {
            geodesic::GeodesicAlgorithmBase* algorithm = all_algorithms[index];        

            algorithm->propagate(sources); // cover the whole mesh

            std::vector<geodesic::SurfacePoint> path;
            for (size_t i = 0; i < targets.size(); ++i) {
                algorithm->trace_back(targets[i], path);
                std::cout << "Results of algorithm " << algorithm->name() << " for path from vertex " << std::to_string(source) << " to " << std::to_string(target)
                 << " (" << std::to_string(path.size() - 1) << " segments, length " << path_length(path) <<"):" << std::endl;
                std::cout << path_rep(path) << std::endl;
            }
        }
    }
    return 0;
}

