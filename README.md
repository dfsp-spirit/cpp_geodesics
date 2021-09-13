# cpp_geodesics
Fast computation of geodesic distances on brain surface meshes in C++.

This repo contains C++ applications to compute geodesic distances on triangular meshes.


## Applications

All applications work with connected triangular meshes in standard mesh formats (PLY, OFF, OBJ) as well as [FreeSurfer](https://freesurfer.net/) brain surface meshes used in computational neuroimaging. See [libfs](https://github.com/dfsp-spirit/libfs) for details.


* `geodpath`: Computes geodesic paths on a mesh from a source vertex to a target vertex. Ouputs coordinates of intermediate points and the total distance. Algorithm can be selected (see `Algorithms` below).
* `geodcircles`: Computes geodesic circle stats and optionally geodesic mean distances on meshes in parallel. See *Ecker, C., Ronan, L., Feng, Y., Daly, E., Murphy, C., Ginestet, C. E., ... & MRC AIMS Consortium. (2013). Intrinsic gray-matter connectivity of the brain in adults with autism spectrum disorder. Proceedings of the National Academy of Sciences, 110(32), 13222-13227. [doi:10.1001/jamapsychiatry.2013.265](https://jamanetwork.com/journals/jamapsychiatry/fullarticle/1393585)* for details on the descriptors. Note that the paper used the Fastmarching toolbox for Matlab by Gabriel Peyre.
* `export_brainmesh`: Exports a FreeSurfer mesh and per-vertex data to a vertex-colored mesh in PLY format (by applying the viridis colormap to the per-vertex data). The colored mesh can then be viewed in standard mesh applications like [MeshLab](https://www.meshlab.net/) or [Blender](https://www.blender.org/).
* `meshneigh_geod`: Compute vertex neighborhoods for all vertices of a mesh and save them to JSON or binary files. This application computes the geodesic neighborhood, i.e., the vertex indices (and distances) of all vertices in a certain geodesic area around each query vertex.
* `meshneigh_edge`: Compute vertex neighborhoods for all vertices of a mesh and save them to JSON or binary files. This application computes the neighborhood using edge distance on the mesh, i.e., the vertex indices of all vertices within graph distance up to the query distance. (This is the adjacency list representation of the mesh for a distance of 1.)


## Building

This requires cmake, a C++ toolchain and OpenMP for maximum performance.

I would recommend `sudo apt install build-essential cmake git`

To build with cmake:

```
cmake .
make
```

You can also build a single app only, e.g., `make geodcircles` or `make geodpath`.


## Usage

Just run the apps without command line arguments to get help. E.g.,

```shell
./geodpath 
===./geodpath -- Compute geodesic path and distance on a mesh. ===
Usage: ./geodpath [<mesh> [<source> [<target>]]]
  <mesh>   : str, path to the input mesh file.
  <source> : int >= 0, the source vertex (0-based index).
  <target> : int >= 0, the target vertex (0-based index).
```


## Algorithms

The applications use algorithms from the the following libraries:

* [VCGLIB](http://vcg.isti.cnr.it/vcglib/): provides an approximative geodesic distance algorithm that travels along the mesh edges (Dijkstra that sums Euclidian distances along edges).
* [geodesic](https://github.com/mojocorp/geodesic/) based on [code.google.com/p/geodesic/geodesic_geo](http://code.google.com/p/geodesic/geodesic_geo): provides 3 geodesic distance algorithms, including:
    - Exact geodesics
    - Dijkstra
    - Subdivision version of Dijkstra that splits edges (becomes Dijkstra algo for `num_extra_splits=0` and approaches exact version with increasing number of splits)



