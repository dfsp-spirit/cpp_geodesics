# cpp_geodesics
Fast computation of geodesic distances and related mesh descriptors on (brain surface) meshes in C++ and OpenMP.


## About 

This repo contains fast C++ applications to compute geodesic distances and related mesh descriptors on triangular meshes. We use them to compute distances on the human cortex in computational neuroimaging, but they work with any connected trimesh. Geodesics computation is computationally expensive and the focus of these applications is mostly on speed.

![Vis](web/fsbrain_geodesic_paths.jpg?raw=true "Geodesic paths on a human brain template. Visualization created with the fsbrain R package.")

**Fig 1: Geodesic paths** *A human brain surface mesh and geodesic paths from a source vertex to several target points on the surface.*


## Applications

All applications that come in this repostory work with connected triangular meshes in standard mesh formats ([PLY](https://en.wikipedia.org/wiki/PLY_(file_format)), [OFF](https://en.wikipedia.org/wiki/OFF_(file_format)), [OBJ](https://de.wikipedia.org/wiki/Wavefront_OBJ)) as well as [FreeSurfer](https://freesurfer.net/) brain surface meshes used in computational neuroimaging. The mesh file format is auto-determined from the file extension. See [libfs](https://github.com/dfsp-spirit/libfs) for details.

The following apps are included:

* `geodcircles`: The core app of this repo. Computes *geodesic circle stats* and optionally *geodesic mean distances* on meshes in parallel.
    * The descriptors serve as proxy measures for *intrinsic wiring costs*, i.e., wiring with the gray matter (parallel to the brain surface) and are:
      - mean separation distance: the average geodesic distance from a vertex to the rest of the surface, as a proxy measure for global wiring costs.
      - geodesic radius: radius of a geodesic circle covering *n* percent of the mesh surface area, as a proxy measure for local, intra-areal wiring costs.
      - geodesic perimeter: perimeter of a geodesic circle covering *n* percent of the mesh surface area, as a proxy measure for local, inter-areal wiring costs.
    * These descriptors are described in mroe detail the following publications:
      - Griffin, L.D. (1994) *The Intrinsic Geometry of the Cerebral Cortex.* Journal of Theoretical Biology, 166(3), 261-273. [doi.org/10.1006/jtbi.1994.1024](https://doi.org/10.1006/jtbi.1994.1024)
      - Ecker, C., Ronan, L., Feng, Y., Daly, E., Murphy, C., Ginestet, C. E., ... & MRC AIMS Consortium. (2013). *Intrinsic gray-matter connectivity of the brain in adults with autism spectrum disorder.* Proceedings of the National Academy of Sciences, 110(32), 13222-13227. [doi.org/10.1073/pnas.1221880110 ](https://doi.org/10.1073/pnas.1221880110)
    *  The *Ecker 2013* paper applied the descriptors to an ASD dataset and used the implementation from the Fastmarching toolbox for Matlab by Gabriel Peyre. The `geodcircles` app in this repository is a C++ implementation using [OpenMP](https://www.openmp.org/) that is an order of magnitude faster on a single CPU core and scales well on multi-core systems. See below for descriptor visualizations.


Utility apps:

* `geodpath`: Simple app that computes [geodesic paths](https://en.wikipedia.org/wiki/Geodesic) on a mesh from a source vertex to a target vertex. It outputs coordinates of intermediate points and the total distance in machine-readbale formats. The algorithm can be selected (see `Algorithms` below).
* `export_brainmesh`: Exports a FreeSurfer mesh and per-vertex data to a vertex-colored mesh in PLY format (by applying the viridis colormap to the per-vertex data). The colored mesh can then be viewed in standard mesh applications like [MeshLab](https://www.meshlab.net/) or [Blender](https://www.blender.org/).
* `meshneigh_geod`: Compute vertex neighborhoods for all vertices of a mesh and save them to JSON, CSV, or [VV binary files](./vv_format.md). This application computes the geodesic neighborhood, i.e., the vertex indices (and distances) of all vertices in a certain geodesic area around each query vertex.
* `meshneigh_edge`: Compute vertex neighborhoods for all vertices of a mesh and save them to JSON, CSV, or VV binary files. This application computes the neighborhood using edge distance on the mesh, i.e., the vertex indices of all vertices within graph distance up to the query distance. (This is the adjacency list representation of the mesh for a distance of 1.)

The utility apps can output to JSON, CSV, or [VV format](./vv_format.md) files.


## Descriptor visualizations

The following images show the three descriptors computed by the `geodcircles` app on a human brain mesh:


### Geodesic mean distance / Mean separation distance

![geod_meandist](web/geod_meandist.png?raw=true "Geodesic mean distance from each vertex to all others.")
**Fig. 2: Mean separation distance** *The geodesic mean distance (aka mean separation distance), computed at each vertex and visualized for a single subject on its white matter surface. This is a proxy measure for global wiring costs.*

### Geodesic perimeter

![geod_perimeter](web/geod_perimeter.png?raw=true "Geodesic perimeter.")
**Fig. 3: Geodesic perimeter** *The perimeter of a circle covering 5% of the total mesh area, computed at each vertex and visualized for a single subject on its white matter surface. This is a proxy measure for local, intra-areal wiring costs.*

### Geodesic radius

![geod_radius](web/geod_radius.png?raw=true "Geodesic radius.")
**Fig 4: Geodesic radius** *The radius of a circle covering 5% of the total mesh area, computed at each vertex and visualized for a single subject on its white matter surface. This is a proxy measure for local, inter-areal wiring costs.*

All figures were produced in R with the [fsbrain](http://github.com/dfsp-spirit/fsbrain) package.

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


## Contact and Author

Please open an issue here on GitHub if you find a bug or have any question or comment.

The programs in this repo were written by [Tim Schäfer](http://rcmd.org/ts). 
