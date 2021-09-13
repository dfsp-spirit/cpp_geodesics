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

    inline void print_info_about_path(const std::vector<SurfacePoint>& path) {
        std::cout << "number of the points in the path = " << path.size()
                << ", length of the path = " << length(path) << std::endl;
    }

}


int main(int argc, char** argv) {
    std::vector<double> points;
    std::vector<unsigned> faces;    

    std::string subject = "fsaverage3";

    
    if(argc > 2) {
        std::cout << "===" << argv[0] << " -- Demo that runs geodesic lib algorithms on brain meshes. ===\n";
        std::cout << "Usage: " << argv[0] << " [<subject>]\n";
        std::cout << "   subject : the subject to use, must be in demo_data\n";
        exit(1);
    }
    if(argc == 2) {
        subject = argv[1];
    }

    std::cout << " Reading FreeSurfer lh.white surface for subject '" + subject + "'...\n";    
    std::string surf_file = "demo_data/subjects_dir/" + subject + "/surf/lh.white";
    fs::Mesh surface;
    fs::read_mesh(&surface, surf_file);

    geodesic::Mesh mesh;
    mesh.initialize_mesh_data(surface.vertices, surface.faces, true); // create 'geodesic' lib mesh data structure

    geodesic::GeodesicAlgorithmExact exact_algorithm(&mesh); // exact algorithm
    geodesic::GeodesicAlgorithmDijkstra dijkstra_algorithm(&mesh); // simplest approximate algorithm: path only allowed on the edges of the mesh
    unsigned const subdivision_level = 3; // three additional vertices per every edge in subdivision algorithm
    geodesic::GeodesicAlgorithmSubdivision subdivision_algorithm(&mesh, subdivision_level); // with subdivision_level=0 this algorithm becomes Dijkstra, with subdivision_level->infinity it becomes exact

    std::vector<geodesic::GeodesicAlgorithmBase*> all_algorithms; // for simplicity, store all possible geodesic algorithms here
    all_algorithms.push_back(&dijkstra_algorithm);
    all_algorithms.push_back(&subdivision_algorithm);
    all_algorithms.push_back(&exact_algorithm);

    std::vector<geodesic::SurfacePoint> sources;
    sources.push_back(geodesic::SurfacePoint(&mesh.vertices()[0])); // one source is located at vertex zero
    sources.push_back(geodesic::SurfacePoint(&mesh.edges()[12])); // second source is located in the middle of edge 12
    sources.push_back(geodesic::SurfacePoint(&mesh.faces()[20])); // third source is located in the middle of face 20

    std::vector<geodesic::SurfacePoint> targets; // same thing with targets
    targets.push_back(geodesic::SurfacePoint(&mesh.vertices().back()));
    targets.push_back(geodesic::SurfacePoint(&mesh.edges()[10]));
    targets.push_back(geodesic::SurfacePoint(&mesh.faces()[3]));

    for (unsigned index = 0; index < all_algorithms.size(); ++index) {
        geodesic::GeodesicAlgorithmBase* algorithm = all_algorithms[index]; // all algorithms are derived from GeodesicAlgorithmBase
        std::cout << std::endl << "results for algorithm " << algorithm->name() << std::endl;

        algorithm->propagate(sources); // cover the whole mesh
        algorithm->print_statistics();

        //-------------first task: compute the pathes to the targets -----
        std::vector<geodesic::SurfacePoint> path;
        for (size_t i = 0; i < targets.size(); ++i) {
            algorithm->trace_back(targets[i], path);
            print_info_about_path(path);
        }

        //-------------second task: for each source, find the furthest vertex that it covers -----
        std::vector<double> max_distance(sources.size(), 0.0); // distance to the furthest vertex that is covered by the given source
        for (size_t i = 0; i < mesh.vertices().size(); ++i) {
            geodesic::SurfacePoint p(&mesh.vertices()[i]);
            double distance;
            unsigned best_source = algorithm->best_source(p, distance);

            max_distance[best_source] = std::max(max_distance[best_source], distance);
        }

        std::cout << std::endl;
        for (size_t i = 0; i < max_distance.size(); ++i) {
            std::cout << "distance to the furthest vertex that is covered by source " << i << " is " << max_distance[i] << std::endl;
        }
    }
    return 0;
}
