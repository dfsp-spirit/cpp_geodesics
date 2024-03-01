# cpp_geodesics
Fast computation of geodesic distances and related mesh descriptors on (brain surface) meshes in C++ and OpenMP.

Paper using the code in this repo: [Leyhausen et al., Biol Psychiatry 2024 95(2):175-186](https://doi.org/10.1016/j.biopsych.2023.06.010)

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.8090921.svg)](https://doi.org/10.5281/zenodo.8090921)
![main](https://github.com/dfsp-spirit/cpp_geodesics/actions/workflows/tests.yml/badge.svg?branch=main)



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
    * These descriptors are described in greater detail the following publications:
      - Griffin, L.D. (1994) *The Intrinsic Geometry of the Cerebral Cortex.* Journal of Theoretical Biology, 166(3), 261-273. [doi.org/10.1006/jtbi.1994.1024](https://doi.org/10.1006/jtbi.1994.1024)
      - Ecker, C., Ronan, L., Feng, Y., Daly, E., Murphy, C., Ginestet, C. E., ... & MRC AIMS Consortium. (2013). *Intrinsic gray-matter connectivity of the brain in adults with autism spectrum disorder.* Proceedings of the National Academy of Sciences, 110(32), 13222-13227. [doi.org/10.1073/pnas.1221880110 ](https://doi.org/10.1073/pnas.1221880110)
      - Johanna Leyhausen, Tim Schäfer, Caroline Gurr, Lisa M. Berg, Hanna Seelemeyer, Charlotte M. Pretzsch, Eva Loth, Bethany Oakley, Jan K. Buitelaar, Christian F. Beckmann, Dorothea L. Floris, Tony Charman, Thomas Bourgeron, Tobias Banaschewski, Emily J.H. Jones, Julian Tillmann, Chris Chatham, The EUAIMS LEAP Group, Declan G. Murphy and Christine Ecker. *Differences in Intrinsic Gray Matter Connectivity and Their Genomic Underpinnings in Autism Spectrum Disorder.* Biological Psychiatry 0006-3223 (2023). [doi.org/10.1016/j.biopsych.2023.06.010](https://doi.org/10.1016/j.biopsych.2023.06.010)
    *  The *Ecker et al. 2013* paper applied the descriptors to an ASD dataset and used the implementation from the Fastmarching toolbox for Matlab by Gabriel Peyre. The `geodcircles` app in this repository is a C++ implementation using [OpenMP](https://www.openmp.org/) that is an order of magnitude faster on a single CPU core *and* scales well on multi-core systems. The implementation in this repository was used in the *Leyhausen et al. 2023* paper.
    *  See below for descriptor visualizations on a brain mesh.

See [README_geodcircles.md](./README_geodcircles.md) for details on using the `geodcircles` application.

Utility apps, used by me for debugging and to generate training data for machine learning algorithms that operate on meshes (maybe useful to others for other purposes?):

* `geodpath`: Simple app that computes [geodesic paths](https://en.wikipedia.org/wiki/Geodesic) on a mesh from a source vertex to a target vertex. It outputs coordinates of intermediate points and the total distance in machine-readable formats. The algorithm can be selected (see `Algorithms` below).
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

```shell
git clone https://github.com/dfsp-spirit/cpp_geodesics
cd cpp_geodesics
cmake .
make
```

In the last step, you can also build a single app only, e.g., `make geodcircles` or `make geodpath`. The resulting binaries are placed into the repo root.

Note for Mac users: Apple's version of the clang compiler that comes with MacOS does not support OpenMP, so you will have to install a suitable compiler and use it to compile these apps. Without OpenMP support, only a single CPU core will be used. Both standard clang or g++, e.g. from Homebrew or MacPorts, work fine.


### Building without cmake

The only officially supported method to build is via cmake, but of course you can just read the [CMakeLists.txt file](./CMakeLists.txt) in combination with your compiler's manual page to construct the commands to compile without cmake. We vendor all dependencies in the repo, so you do not need to worry about installing other things. E.g., to manually compile `geodcircles` with the GNU C++ compiler, you would do:

```shell
g++ -Isrc/common -I src/common_vcg -Ithird_party/libfs -Ithird_party/vcglib -Ithird_party/vcglib/eigenlib -Ithird_party/spline -I third_party/tinycolormap third_party/vcglib/wrap/ply/plylib.cpp src/geodcircles/main.cpp -o geodcircles
```

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

The most important application in this repo is `geodcircles`.

### Running a full geodcircles demo computation on the included demo data

This repository contains some down-sampled [demonstartion data](./demo_data/). Here is how to run the `geodcircles` application on the demo data (after you have compiled it as explained in the section *Building* above):

```shell
./geodcircles demo_data/subjects_dir/subjects_fsaverage3.txt demo_data/subjects_dir/
```

This will perform the computations on all subjects listed in the file [demo_data/subjects_dir/subjects_fsaverage3.txt](./demo_data/subjects_dir/subjects_fsaverage3.txt) and write the output files into the respective `surf/` sub directory of each subject. The demo data in the directory ```./demo_data/subjects_dir/``` is organized exactly in the way FreeSurfer writes its output.

The output files are written in FreeSurfer `curv` format by default, you can use command line arguments to also write in FreeSurfer `MGH` format.

Note: Both formats can be read in various programming languages, e.g., in Matlab using the functions that comes with FresSurfer in the `matlab/` sub directory of the installation folder, in Python with the [nibabel](https://nipy.org/nibabel/) package, in R with the `freesurferformats` package ([repo](https://github.com/dfsp-spirit/freesurferformats), [CRAN](https://CRAN.R-project.org/package=freesurferformats)), in C++ with [libfs](https://github.com/dfsp-spirit/libfs). I have also written libraries for Rust, Java, Julia and Go that can handle the formats, see the [list of my repos](https://github.com/dfsp-spirit?tab=repositories) here on my Github account.


See [README_geodcircles.md](./README_geodcircles.md) for details on using the `geodcircles` application.


## Algorithms

The applications use algorithms from the the following libraries:

* [VCGLIB](http://vcg.isti.cnr.it/vcglib/): provides an approximative geodesic distance algorithm that travels along the mesh edges (Dijkstra that sums Euclidian distances along edges).
* [geodesic](https://github.com/mojocorp/geodesic/) based on [code.google.com/p/geodesic/geodesic_geo](http://code.google.com/p/geodesic/geodesic_geo): provides 3 geodesic distance algorithms, including:
    - Exact geodesics
    - Dijkstra
    - Subdivision version of Dijkstra that splits edges (becomes Dijkstra algo for `num_extra_splits=0` and approaches exact version with increasing number of splits)

FreeSurfer data is read and written with [libfs](https://github.com/dfsp-spirit/libfs).


## Unit tests

To build the unit tests, simply follow the instructions in the section `Building` above. This creates several binaries, including the `cpp_geodesic_tests` binary. Execute it to run the tests:

```shell
./cpp_geodesic_tests
```


## Contact and Author

Please open an issue here on GitHub if you find a bug or have any question or comment.

The programs in this repo were written by [Tim Schäfer](https://ts.rcmd.org).

Thanks to all the great people who wrote the [dependencies](./third_party/) (and their dependencies).
