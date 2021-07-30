# cpp_geodesics
Fast computation of geodesic distances on brain surface meshes in C++.

This repo contains C++ applications to compute geodesic distances on triangular meshes.

## Building

This requires cmake, a C++ toolchain and OpenMP for maximum performance.

I would recommend `sudo apt install build-essential cmake git`

To build with cmake:

```
cmake .
make
```

You can also build a single app only, e.g., `make geodesic_vcg` or `make geodesic_geo`.

## Algorithms

The applications use the following algorithms:

* geodesic_vcg: The VCGLIB implementation, which does Dijkstra and sums Euclidian distances along edges.
* geodesic_geo: Several algorithms, including:
    - Exact geodesics
    - Dijkstra
    - Subdivision version of Dijkstra that splits edges (becomes Dijkstra algo for num_extra_splits=1 and approaches exact version with increasing number of splits)



