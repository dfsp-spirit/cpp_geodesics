# cpp_geodesics
Fast computation of geodesic distances and related mesh descriptors on (brain surface) meshes in C++ and OpenMP.


## About 

This repo contains fast C++ applications to compute geodesic distances and related mesh descriptors on triangular meshes. We use them to compute distances on the human cortex in computational neuroimaging, but they work with any connected trimesh. Geodesics computation is computationally expensive and the focus of these applications is mostly on speed.

![Vis](web/fsbrain_geodesic_paths.jpg?raw=true "Geodesic paths on a human brain template. Visualization created with the fsbrain R package.")

## Applications

All applications that come in this repostory work with connected triangular meshes in standard mesh formats ([PLY](https://en.wikipedia.org/wiki/PLY_(file_format)), [OFF](https://en.wikipedia.org/wiki/OFF_(file_format)), [OBJ](https://de.wikipedia.org/wiki/Wavefront_OBJ)) as well as [FreeSurfer](https://freesurfer.net/) brain surface meshes used in computational neuroimaging. The mesh file format is auto-determined from the file extension. See [libfs](https://github.com/dfsp-spirit/libfs) for details.

The following apps are included:

* `geodcircles`: Core app of this repo. Computes *geodesic circle stats* and optionally *geodesic mean distances* on meshes in parallel. These two descriptors are described in *Ecker, C., Ronan, L., Feng, Y., Daly, E., Murphy, C., Ginestet, C. E., ... & MRC AIMS Consortium. (2013). Intrinsic gray-matter connectivity of the brain in adults with autism spectrum disorder. Proceedings of the National Academy of Sciences, 110(32), 13222-13227. [doi:10.1001/jamapsychiatry.2013.265](https://jamanetwork.com/journals/jamapsychiatry/fullarticle/1393585)*. The paper used the implementation from the Fastmarching toolbox for Matlab by Gabriel Peyre. The `geodcircles` app in this repository is a C++ implementation using [OpenMP](https://www.openmp.org/) that is an order of magnitude faster on a single CPU core and scales well on multi-core systems.
* `geodpath`: Simple app that computes [geodesic paths](https://en.wikipedia.org/wiki/Geodesic) on a mesh from a source vertex to a target vertex. It outputs coordinates of intermediate points and the total distance in machine-readbale formats. The algorithm can be selected (see `Algorithms` below).


Utility apps:

* `export_brainmesh`: Exports a FreeSurfer mesh and per-vertex data to a vertex-colored mesh in PLY format (by applying the viridis colormap to the per-vertex data). The colored mesh can then be viewed in standard mesh applications like [MeshLab](https://www.meshlab.net/) or [Blender](https://www.blender.org/).
* `meshneigh_geod`: Compute vertex neighborhoods for all vertices of a mesh and save them to JSON or binary files. This application computes the geodesic neighborhood, i.e., the vertex indices (and distances) of all vertices in a certain geodesic area around each query vertex.
* `meshneigh_edge`: Compute vertex neighborhoods for all vertices of a mesh and save them to JSON or binary files. This application computes the neighborhood using edge distance on the mesh, i.e., the vertex indices of all vertices within graph distance up to the query distance. (This is the adjacency list representation of the mesh for a distance of 1.)

The utility apps can output to JSON or a dense, binary custom format.


## Building

This requires cmake, a C++ toolchain and OpenMP for maximum performance.

I would recommend `sudo apt install build-essential cmake git`

To build with cmake:

```
git clone https://github.com/dfsp-spirit/cpp_geodesics
cd cpp_geodesics
cmake .
make
```

In the last step, you can also build a single app only, e.g., `make geodcircles` or `make geodpath`. The resulting binaries are placed into the repo root.

Note for Mac users: Apple's version of the clang compiler that comes with MacOS does not support OpenMP, so you will have to install a suitable compiler and use it to compile these apps. Without OpenMP support, only a single CPU core will be used. Both standard clang or g++, e.g. from Homebrew or MacPorts, work fine.


## Usage

Just run the apps without command line arguments to get help. E.g.,

```shell
./geodpath 
===./geodpath -- Compute geodesic path and distance on a mesh. ===
Usage: ./geodpath <mesh> [<source> [<target>]]
  <mesh>   : str, path to the input mesh file.
  <source> : int >= 0, the source vertex (0-based index). Defaults to 0.
  <target> : int >= 0, the target vertex (0-based index). Defaults to 100.
```


## Algorithms

The applications use algorithms from the the following libraries:

* [VCGLIB](http://vcg.isti.cnr.it/vcglib/): provides an approximative geodesic distance algorithm that travels along the mesh edges (Dijkstra that sums Euclidian distances along edges).
* [geodesic](https://github.com/mojocorp/geodesic/) based on [code.google.com/p/geodesic/geodesic_geo](http://code.google.com/p/geodesic/geodesic_geo): provides 3 geodesic distance algorithms, including:
    - Exact geodesics
    - Dijkstra
    - Subdivision version of Dijkstra that splits edges (becomes Dijkstra algo for `num_extra_splits=0` and approaches exact version with increasing number of splits)

FreeSurfer data is read and written with [libfs](https://github.com/dfsp-spirit/libfs).


