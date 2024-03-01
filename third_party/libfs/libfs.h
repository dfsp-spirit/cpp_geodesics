#pragma once

#include <iostream>
#include <climits>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <cstdint>


#define LIBFS_VERSION "0.3.4"

/// @file
///
/*! \mainpage The libfs API documentation
 *
 * \section intro_sec Introduction
 *
 * Welcome to the API documentation for libfs, a header-only C++11 library to read and write FreeSurfer neuroimaging data.
 *
 * All relevant functions are in the file include/libfs.h and only a few utility functions are class
 * members, so the best place to start is to open the documentation for libfs.h in the Files section above.
 *
 * \subsection intro-examples A note on the API doc examples
 *
 * The examples in the doc strings of the libfs.h functions usually only show data preparation and the function call itself. Typically you
 * will get a working program out of them by wrapping them into something like:
 *
 * @code
 * #include "libfs.h"
 * #include <string>
 * #include <iostream>
 * #include <vector>
 * // maybe more includes for some examples here.
 *
 * int main(int argc, char** argv) {
 *      // Demo code goes here
 * }
 * @endcode
 *
 * To see full demo programs and compilation instructions, check the <a href="https://github.com/dfsp-spirit/libfs/tree/main/examples">examples/ directory</a> in the GitHub repository linked below.
 *
 * \subsection logging Logging with libfs
 *
 * You can define the output produced by libfs from your application. To do so,
 * `#define` _one_ of the following debug levels in your application, *before* including 'libfs.h':
 *
 *  - `LIBFS_DBG_CRITICAL`     // print only critical errors that will raise an expection and most likely cause application to stop (unless caught).
 *  - `LIBFS_DBG_ERROR`        // prints errors (and more severe things).
 *  - `LIBFS_DBG_WARNING`      // the default, prints warnings (and more severe things).
 *  - `LIBFS_DBG_INFO`         // prints info messages, like what is currently being done.
 *  - `LIBFS_DBG_VERBOSE`      // prints info messages inside loops, may considerable slow down apps and litter stdout.
 *  - `LIBFS_DBG_EXCESSIVE`    // prints info messages in nested loops, will considerable slow down apps and quickly litter stdout.
 *
 *
 * Things you should know about logging and controlling libfs output:
 *
 *   - The debug levels are ordered in the list above, and defining a single one will automatically enable
 * all levels of higher importance (e.g., defining `LIBFS_DBG_WARNING` also enables `LIBFS_DBG_ERROR` and `LIBFS_DBG_CRITICAL`).
 *   - If you define nothing at all, libfs defaults to `LIBFS_DBG_WARNING`.
 *   - If you do not want any ouput from libfs, define `LIBFS_DBG_NONE`. This is not recommended though, as it completely disables
 *     all output, including critical error messages. This means that your application may terminate without any message,
 *     and is only advisable if you are very sure that you catch all possible exceptions and then produce an error message
 *     for users in your application code.
 *   - Currently all debug output goes to `stdout`, i.e., typically to the terminal.
 *
 *
 * \subsection intro-website The libfs project website
 *
 * The project page for libfs can be found at https://github.com/dfsp-spirit/libfs. It contains information on all documentation available for libfs.
 *
 *
 */


// Set apptag (printed as prefix of debug messages) for debug messages.
// Users can overwrite this by defining LIBFS_APPTAG before including 'libfs.h'.
#ifndef LIBFS_APPTAG
#define LIBFS_APPTAG "[libfs] "
#endif

// Set default.
#define LIBFS_DBG_WARNING

// If the user wants something below our default, remove our default.
#ifdef LIBFS_DBG_NONE
#undef LIBFS_DBG_WARNING
#endif

#ifdef LIBFS_DBG_CRITICAL
#undef LIBFS_DBG_WARNING
#endif

#ifdef LIBFS_DBG_ERROR
#undef LIBFS_DBG_WARNING
#endif

// Ensure that the user does not have to define all debug levels
// up to the one they actually want, by defining all lower ones for them.
#ifdef LIBFS_DBG_EXCESSIVE
#define LIBFS_DBG_VERBOSE
#endif

#ifdef LIBFS_DBG_VERBOSE
#define LIBFS_DBG_INFO
#endif

#ifdef LIBFS_DBG_INFO
#define LIBFS_DBG_WARNING
#endif

#ifdef LIBFS_DBG_WARNING
#define LIBFS_DBG_ERROR
#endif

#ifdef LIBFS_DBG_ERROR
#define LIBFS_DBG_CRITICAL
#endif

// End of debug handling.

namespace fs {

  namespace util {

    /// @brief  Cross-platform wrapper for localtime_r and localtime_s.
    /// @param time the time to convert to an std::tm struct.
    /// @return the std::tm struct.
    tm _localtime(const std::time_t& time) {
      std::tm tm_snapshot;
      #if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
        ::localtime_s(&tm_snapshot, &time);
      #else
        ::localtime_r(&time, &tm_snapshot); // POSIX
      #endif
        return tm_snapshot;
    }

    /// @brief Get current time as string, e.g. for log messages.
    /// @param t the timepoint to format as a string, typically `std::system_clock::now()`.
    /// @return the formatted time string.
    /// #### Examples
    ///
    /// @code
    /// std::string time_rep = fs::util::time_tag(std::chrono::system_clock::now());
    /// @endcode
    std::string time_tag(std::chrono::system_clock::time_point t) {
      auto as_time_t = std::chrono::system_clock::to_time_t(t);
      struct tm tm;
      char time_buffer[64];
      //if (::gmtime_r(&as_time_t, &tm)) {
      tm = _localtime(as_time_t);
      if (std::strftime(time_buffer, sizeof(time_buffer), "%F %T", &tm)) {
        return std::string{time_buffer};
      }
      throw std::runtime_error("Failed to get current date as string");
    }

    /// Logging threshold for critical messages.
    const std::string LOGTAG_CRITICAL = "CRITICAL";

    /// Logging threshold for error messages.
    const std::string LOGTAG_ERROR = "ERROR";

    /// Logging threshold for warning messages.
    const std::string LOGTAG_WARNING = "WARNING";

    /// Logging threshold for warning messages.
    const std::string LOGTAG_INFO = "INFO";

    /// Logging threshold for warning messages.
    const std::string LOGTAG_VERBOSE = "VERBOSE";

    /// Logging threshold for warning messages.
    const std::string LOGTAG_EXCESSIVE = "EXCESSIVE";

    /// @brief Log a message, goes to stdout.
    /// @param message the message to be logged.
    /// @param loglevel the log level, one of `fs::util::LOGTAG_*`.
    inline void log(std::string const & message, std::string const loglevel = "INFO") {
      std::cout << LIBFS_APPTAG << "[" << loglevel << "] [" << fs::util::time_tag(std::chrono::system_clock::now()) << "] " << message << "\n";
    }

    /// @brief Check whether a string ends with the given suffix.
    /// @private
    ///
    /// #### Examples
    ///
    /// @code
    /// bool ev = fs::util::ends_with("freesurfer", "surfer"); // true
    /// @endcode
    inline bool ends_with(std::string const & value, std::string const & suffix) {
        if (suffix.size() > value.size()) return false;
        return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
    }

    /// @brief Check whether a string ends with one of the given suffixes.
    /// @private
    ///
    /// #### Examples
    ///
    /// @code
    /// bool ev = fs::util::ends_with("freesurfer", {"surfer", "not"}); // true
    /// @endcode
    inline bool ends_with(std::string const & value, std::initializer_list<std::string> suffixes) {
      for (auto suffix : suffixes) {
        if (ends_with(value, suffix)) {
          return true;
        }
      }
      return false;
    }


    /// @brief Turn 1D vector into 2D vector.
    /// @param values the input 1D vector.
    /// @param num_cols number of columns for the returned 2D vector.
    /// @return 2D vector with `num_cols` columns.
    /// @private
    ///
    /// #### Examples
    ///
    /// @code
    /// std::vector<float> input = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    /// std::vector<std::vector<float>> res = fs::util::v2d(input, 2);
    /// @endcode
    template <typename T>
    std::vector<std::vector <T>> v2d(std::vector<T> values, size_t num_cols) {
      std::vector<std::vector <T>> result;
      for (std::size_t i = 0; i < values.size(); ++i) {
          if (i % num_cols == 0) {
            result.resize(result.size() + 1);
          }
          result[i / num_cols].push_back(values[i]);
      }
      return result;
    }

    /// @brief Flatten 2D vector.
    /// @param values the input 2D vector.
    /// @return 1D vector.
    ///
    /// #### Examples
    ///
    /// @code
    /// std::vector<float> input = { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
    /// std::vector<std::vector<float>> res = fs::util::v2d(input, 2);
    /// @endcode
    template <typename T>
    std::vector<T> vflatten(std::vector<std::vector <T>> values) {
      size_t total_size = 0;
      for (std::size_t i = 0; i < values.size(); i++) {
        total_size += values[i].size();
      }

      std::vector <T> result = std::vector<T>(total_size);
      size_t cur_idx = 0;
      for (std::size_t i = 0; i < values.size(); i++) {
        for (std::size_t j = 0; j < values[i].size(); j++) {
          result[cur_idx] = values[i][j];
          cur_idx++;
        }
      }
      return result;
    }

    /// @brief Check whether a string starts with the given prefix.
    /// @private
    /// @note This is a private function, users should call the overloaded version that accepts
    ///       a vector of prefixes instead.
    ///
    /// #### Examples
    ///
    /// @code
    /// bool ev = fs::util::starts_with("freesurfer", "free"); // true
    /// @endcode
    inline bool starts_with(std::string const & value, std::string const & prefix) {
        if (prefix.length() > value.length()) return false;
        return value.rfind(prefix, 0) == 0;
    }

    /// @brief Check whether a string starts with one of the given prefixes.
    /// @param value the string for which to check whether it starts with any of the prefixes
    /// @param prefixes the prefixes to consider
    /// @returns whether the string starts with one of the prefixes.
    ///
    /// #### Examples
    ///
    /// @code
    /// bool ev = fs::util::starts_with("freesurfer", {"free", "not"}); // true
    /// @endcode
    inline bool starts_with(std::string const & value, std::initializer_list<std::string> prefixes) {
      for (auto prefix : prefixes) {
        if (starts_with(value, prefix)) {
          return true;
        }
      }
      return false;
    }

    /// @brief Check whether a file exists (can be read) at given path.
    /// @details You should not rely on this as a pre-check when considering to open a file due
    ///          to race conditions, just try-catch open in that case. This is intended to check
    ///          whether a certain software run succeeded, by checking whether the key expected
    ///          output files exist.
    /// @param name the filename that should be checked.
    /// #### Examples
    ///
    /// @code
    /// bool exists = fs::util::file_exists("./study1/subject1/label/lh.aparc.annot");
    /// @endcode
    inline bool file_exists(const std::string& name) {
      if (FILE *file = fopen(name.c_str(), "r")) {
          fclose(file);
          return true;
      } else {
          return false;
      }
    }


    /// @brief Construct a UNIX file system path from the given path_components.
    /// @details Any trailing or leading slash (path_sep) will be stripped from the individual components and replaced with a single one between two components. If the first path component started with a slash, that slash will be kept (absolute paths are left intact).
    /// @param path_components init list of strings, the path components
    /// @param path_sep path separator to use, typically `/` on Unix-based system.
    /// @throws std::invalid_argument on empty
    /// @returns string representation of the path, using the `path_sep`.
    ///
    /// #### Examples
    ///
    /// @code
    /// std::string p = fs::util::fullpath({"path", "to", "file.txt"});
    /// // Gives: "path/to/file.txt"
    /// std::string p = fs::util::fullpath({"/path", "to", "file.txt"});
    /// // Gives: "/path/to/file.txt"
    /// @endcode
    std::string fullpath( std::initializer_list<std::string> path_components, std::string path_sep = std::string("/") ) {
      std::string fp;
      if(path_components.size() == 0) {
          throw std::invalid_argument("The 'path_components' must not be empty.");
      }

      std::string comp;
      std::string comp_mod;
      size_t idx = 0;
      for(auto comp : path_components) {
        comp_mod = comp;
        if(idx != 0) { // We keep a leading slash intact for the first element (absolute path).
          if (starts_with(comp, path_sep)) {
            comp_mod = comp.substr(1, comp.size()-1);
          }
        }

        if(ends_with(comp_mod, path_sep)) {
            comp_mod = comp_mod.substr(0, comp_mod.size()-1);
        }

        fp += comp_mod;
        if(idx < path_components.size()-1) {
          fp += path_sep;
        }
        idx++;
      }
      return fp;
    }

    /// @brief Write the given text representation (any string) to a file.
    /// @param filename the file to which to write, will be overwritten if exists
    /// @param rep the string to write to the file
    /// @throws std::runtime_error if the file cannot be opened.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::util::str_to_file("thoughts.txt", "blah, blah, blah");
    /// @endcode
    void str_to_file(const std::string& filename, const std::string rep) {
      std::ofstream ofs;
      ofs.open(filename, std::ofstream::out);
      #ifdef LIBFS_DBG_VERBOSE
      std::cout << LIBFS_APPTAG << "Opening file '" << filename << "' for writing.\n";
      #endif
      if(ofs.is_open()) {
          ofs << rep;
          ofs.close();
      } else {
          throw std::runtime_error("Unable to open file '" + filename + "' for writing.\n");
      }
    }

  }  // End namespace util.


  // MRI data types, used by the MGH functions.

  /// MRI data type representing an 8 bit unsigned integer.
  const int MRI_UCHAR = 0;

  /// MRI data type representing a 32 bit signed integer.
  const int MRI_INT = 1;

  /// MRI data type representing a 32 bit float.
  const int MRI_FLOAT = 3;

  /// MRI data type representing a 16 bit signed integer.
  const int MRI_SHORT = 4;

  // Forward declarations.
  int _fread3(std::istream&);
  template <typename T> T _freadt(std::istream&);
  std::string _freadstringnewline(std::istream&);
  std::string _freadfixedlengthstring(std::istream&, size_t, bool);
  bool _ends_with(std::string const &fullString, std::string const &ending);
  size_t _vidx_2d(size_t, size_t, size_t);
  struct MghHeader;

  /// @brief Models a triangular mesh, used for brain surface meshes.
  ///
  /// @details Represents a vertex-indexed mesh. The `n` vertices are stored as 3D point coordinates (x,y,z) in a vector
  /// of length `3n`, in which 3 consecutive values represent the x, y and z coordinate of the same vertex.
  /// The `m` faces are stored as a vector of `3m` integers, where 3 consecutive values represent the 3 vertices (by index)
  /// making up the respective face. Vertex indices are 0-based.
  /// #### Examples
  ///
  /// @code
  /// fs::Mesh surface = fs::Mesh::construct_cube();
  /// size_t nv = surface.num_vertices(); // 8
  /// auto first_face_verts = surface.face_vertices(0);
  /// int first_face_third_vert = surface.fm_at(0, 2);
  /// size_t nf = surface.num_faces();
  /// size_t nv = surface.num_vertices();
  /// surface.to_obj("cube_out.obj");
  /// @endcode
  struct Mesh {

    /// Construct a Mesh from the given vertices and faces.
    Mesh(std::vector<float> cvertices, std::vector<int32_t> cfaces) {
      vertices = cvertices; faces = cfaces;
    }

    // Construct from 2D vectors (Nx3).
    Mesh(std::vector<std::vector<float>> cvertices, std::vector<std::vector<int32_t>> cfaces) {
      vertices = util::vflatten(cvertices); faces = util::vflatten(cfaces);
    }

    /// Construct an empty Mesh.
    Mesh() {}

    std::vector<float> vertices;  ///< *n x 3* vector of the *x*,*y*,*z* coordinates for the *n* vertices. The x,y,z coordinates for a single vertex form consecutive entries.
    std::vector<int32_t> faces;  ///< *n x 3* vector of the 3 vertex indices for the *n* triangles or faces. The 3 vertices of a single face form consecutive entries.

    /// @brief Construct and return a simple cube mesh.
    /// @return fs::Mesh instance representing a cube.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// size_t nv = surface.num_vertices();  // 8
    /// size_t nf = surface.num_faces();     // 12
    /// @endcode
    static fs::Mesh construct_cube() {
      fs::Mesh mesh;
      mesh.vertices = { 1.0, 1.0, 1.0,
                        1.0, 1.0, -1.0,
                        1.0, -1.0, 1.0,
                        1.0, -1.0, -1.0,
                      -1.0, 1.0, 1.0,
                      -1.0, 1.0, -1.0,
                      -1.0, -1.0, 1.0,
                      -1.0, -1.0, -1.0 };
      mesh.faces = { 0, 2, 3,
                    3 ,1, 0,
                    4, 6, 7,
                    7, 5, 4,
                    0, 4, 5,
                    5, 1, 0,
                    2, 6, 7,
                    7, 3, 2,
                    0, 4, 6,
                    6, 2, 0,
                    1, 5, 7,
                    7, 3, 1 };
      return mesh;
    }

    /// @brief Construct and return a simple pyramidal mesh.
    /// @details This constructs a right square pyramid with base edge length 1 and height 1. Think of the Great Pyramid of Giza.
    /// @return fs::Mesh instance representing a 4-sided pyramid.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_pyramid();
    /// size_t nv = surface.num_vertices();  // 5
    /// size_t nf = surface.num_faces();     // 6
    /// @endcode
    static fs::Mesh construct_pyramid() {
      fs::Mesh mesh;
      mesh.vertices = { 0.0, 0.0, 0.0, // start with 4x base
                        0.0, 1.0, 0.0,
                        1.0, 1.0, 0.0,
                        1.0, 0.0, 0.0,
                        0.5, 0.5, 1.0 }; // apex
      mesh.faces = { 0, 2, 1, // start with 2 base faces
                     0, 3, 2,
                     0, 4, 1, // now the 4 wall faces
                     1, 4, 2,
                     3, 2, 4,
                     0, 3, 4 };
      return mesh;
    }

    /// @brief Construct and return a simple planar grid mesh.
    /// @details This is a 2D rectangular grid embedded in 3D. Each rectangular cell consists of 2 triangular faces. The height (z coordinate) for all vertices is `0.0`.
    /// @param nx number of vertices in x direction
    /// @param ny number of vertices in y direction
    /// @param distx distance between vertices in x direction
    /// @param disty distance between vertices in y direction
    /// @return fs::Mesh instance representing a flat grid.
    /// @throws std::invalid_argument error if `nx` or `ny` are `< 2`.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_grid(4, 5);
    /// size_t nv = surface.num_vertices();  // 4*5 = 20
    /// size_t nf = surface.num_faces();     // (4-1)*(5-1)*2 = 24;
    /// @endcode
    static fs::Mesh construct_grid(const size_t nx = 4, const size_t ny = 5, const float distx = 1.0, const float disty = 1.0) {
      if(nx < 2 || ny < 2) {
        throw std::runtime_error("Parameters nx and ny must be at least 2.");
      }
      fs::Mesh mesh;
      size_t num_vertices = nx * ny;
      size_t num_faces = ((nx - 1) * (ny - 1)) * 2;
      std::vector<float> vertices;
      vertices.reserve(num_vertices * 3);
      std::vector<int> faces;
      faces.reserve(num_faces * 3);

      // Create vertices.
      float cur_x, cur_y, cur_z;
      cur_x = cur_y = cur_z = 0.0;
      for(size_t i = 0; i < nx; i++) {
        for(size_t j = 0; j < ny; j++) {
          vertices.push_back(cur_x);
          vertices.push_back(cur_y);
          vertices.push_back(cur_z);
          cur_y += disty;
        }
        cur_x += distx;
      }

      // Create faces.
      for(size_t i = 0; i < num_vertices; i++) {
        if((i+1) % ny == 0 || i >= num_vertices - ny) {
          // Do not use the last ones in row or column as source.
          continue;
        }
        // Add the upper left triangle of this grid cell.
        faces.push_back(int(i));
        faces.push_back(int(i + ny + 1));
        faces.push_back(int(i + 1));
        // Add the lower right triangle of this grid cell.
        faces.push_back(int(i));
        faces.push_back(int(i + ny + 1));
        faces.push_back(int(i + ny));
      }

      mesh.vertices = vertices;
      mesh.faces = faces;
      return mesh;
    }


    /// @brief Return string representing the mesh in Wavefront Object (.obj) format.
    /// @return Wavefront Object string representation of the mesh, including vertices and faces.
    /// @see fs::Mesh::to_obj_file is a shortcut if you want to export the string representation to a file.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// std::str mesh_repr_off = surface.to_obj();
    /// @endcode
    std::string to_obj() const {
      std::stringstream objs;
      for(size_t vidx=0; vidx<this->vertices.size(); vidx+=3) { // vertex coords
        objs << "v " << vertices[vidx] << " " << vertices[vidx+1] << " " << vertices[vidx+2] << "\n";
      }
      for(size_t fidx=0; fidx<this->faces.size(); fidx+=3) { // faces: vertex indices, 1-based
        objs << "f " << faces[fidx]+1 << " " << faces[fidx+1]+1 << " " << faces[fidx+2]+1 << "\n";
      }
      return(objs.str());
    }

    /// @brief Return adjacency matrix representation of this mesh.
    /// @return boolean 2D matrix, where true means an edge between the respective vertex pair exists, and false mean it does not.
    /// @see fs::Mesh::to_rep_adjlist gives you an adjacency list instead.
    /// @note This requires a lot of memory for large meshes.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// std::vector<std::vector<bool>> adjm = surface.as_adjmatrix();
    /// @endcode
    std::vector<std::vector<bool>> as_adjmatrix() const {
      std::vector<std::vector<bool>> adjm = std::vector<std::vector<bool>>(this->num_vertices(), std::vector<bool>(this->num_vertices(), false));
      for(size_t fidx=0; fidx<this->faces.size(); fidx+=3) { // faces: vertex indices
        adjm[faces[fidx]][faces[fidx+1]] = true;
        adjm[faces[fidx+1]][faces[fidx]] = true;
        adjm[faces[fidx+1]][faces[fidx+2]] = true;
        adjm[faces[fidx+2]][faces[fidx+1]] = true;
        adjm[faces[fidx+2]][faces[fidx]] = true;
        adjm[faces[fidx]][faces[fidx+2]] = true;
      }
      return adjm;
    }

    /// @brief Hash function for 2-tuples of `<size_t, sizt_t>`, used to hash an edge of a graph or mesh.
    struct _tupleHashFunction {
      size_t operator()(const std::tuple<size_t , size_t>&x) const {
        return std::get<0>(x) ^ std::get<1>(x);
      }
    };

    /// @brief Datastructure for storing, and quickly querying the existence of, mesh edges.
    /// @details This is an unordered set of 2-tuples, where each tuple represents an edge, given as a pair of vertex indices. Each edge occurs twice in the list, once as `make_tuple(i,j)` and once as `make_tuple(j,i)`. Use the API of `std::unordered_set` to interact with it.
    typedef std::unordered_set<std::tuple<size_t, size_t>, _tupleHashFunction> edge_set;

    /// @brief Return edge list representation of this mesh.
    /// @note While this mesh or graph representation is typically known as an edge list, this function actually returns a set.
    /// @return an `fs::Mesh::edge_set`, i.e., an unordered set of 2-tuples, where each tuple represents an edge, given as a pair of vertex indices. Each edge occurs twice in the list, once as `make_tuple(i,j)` and once as `make_tuple(j,i)`.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// edge_set edges = surface.as_edgelist();
    /// size_t num_undirected_edges = edg es.size() / 2;
    /// @endcode
    edge_set as_edgelist() const {
      edge_set edges;
      for(size_t fidx=0; fidx<this->faces.size(); fidx+=3) { // faces: vertex indices
        edges.insert(std::make_tuple(faces[fidx], faces[fidx+1]));
        edges.insert(std::make_tuple(faces[fidx+1], faces[fidx]));

        edges.insert(std::make_tuple(faces[fidx+1], faces[fidx+2]));
        edges.insert(std::make_tuple(faces[fidx+2], faces[fidx+1]));

        edges.insert(std::make_tuple(faces[fidx], faces[fidx+2]));
        edges.insert(std::make_tuple(faces[fidx+2], faces[fidx]));
      }
      return edges;
    }

    /// @brief Return adjacency list representation of this mesh.
    /// @param via_matrix whether the computation should be done via an  step involving an adjacency matrix, or via an edge set. Leaving this at `true` temporarily requires a lot of memory for large meshes, but is faster.
    /// @return vector of vectors, where the outer vector has size this->num_vertices. The inner vector at index N contains the M neighbors of vertex n, as vertex indices.
    /// @see fs::Mesh::to_rep_adjmatrix gives you an adjacency matrix instead.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// std::vector<std::vector<size_t>> adjl = surface.as_adjlist();
    /// std::vector<std::vector<size_t>> adjl1 = surface.as_adjlist(true);
    /// @endcode
    std::vector<std::vector<size_t>> as_adjlist(const bool via_matrix=true) const {
      if(! via_matrix) {
        return(this->_as_adjlist_via_edgeset());
      }
      std::vector<std::vector<bool>> adjm = this->as_adjmatrix();
      std::vector<std::vector<size_t>> adjl = std::vector<std::vector<size_t>>(this->num_vertices(), std::vector<size_t>());
      size_t nv = adjm.size();
      for (size_t i = 0; i < nv; i++) {
          for (size_t j = i+1; j < nv; j++) {
              if (adjm[i][j] == true) {
                  adjl[i].push_back(j);
                  adjl[j].push_back(i);
              }
          }
      }
      return adjl;
    }

    /// @brief Return adjacency list representation of this mesh via edge list.
    /// @return vector of vectors, where the outer vector has size this->num_vertices. The inner vector at index N contains the M neighbors of vertex n, as vertex indices.
    /// @see fs::Mesh::to_rep_adjmatrix gives you an adjacency matrix instead.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// std::vector<std::vector<size_t>> adjl = surface.as_adjlist();
    /// @endcode
    std::vector<std::vector<size_t>> _as_adjlist_via_edgeset() const {
      edge_set edges = this->as_edgelist();
      std::vector<std::vector<size_t>> adjl = std::vector<std::vector<size_t>>(this->num_vertices(), std::vector<size_t>());
      for (const std::tuple<size_t, size_t> &e: edges) {
        adjl[std::get<0>(e)].push_back(std::get<1>(e));
      }
      return adjl;
    }

    /// @brief Smooth given per-vertex data using nearest neighbor smoothing.
    /// @param pvd vector of per-vertex data values, one value per mesh vertex.
    /// @param num_iter number of iterations of smoothing to perform.
    /// @param via_matrix passed on to `this->as_asjlist()`, whether to construct the adjacency list of the mesh using an intermediate step involving an adjacency matrix, as opposed to using an edge set. The latter is slower but requires less memory.
    /// @param with_nan whether you need support for NAN values in `pvd`. A bit slower if active. Ignored if `detectnan` is `true`.
    /// @param detect_nan whether to auto-detect presence of NAN values, ignoring the setting of `with_nan`.
    /// @return vector of smoothed per-vertex data values, same length as `pvd` param.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// std::vector<float> pvd = {1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7};
    /// std::vector<float> pvd_smooth = surface.smooth_pvd_nn(pvd, 2);
    /// @endcode
    std::vector<float> smooth_pvd_nn(const std::vector<float> pvd, const size_t num_iter=1, const bool via_matrix=true, const bool with_nan=true, const bool detect_nan=true) const {

      const std::vector<std::vector<size_t>> adjlist = this->as_adjlist(via_matrix);
      return fs::Mesh::smooth_pvd_nn(adjlist, pvd, num_iter, with_nan, detect_nan);
    }

    /// @brief Smooth given per-vertex data using nearest neighbor smoothing based on adjacency list mesh represenation.
    /// @param mesh_adj the mesh, given as an adjacency list. The outer vector has size num_vertices, and the inner vectors sizes are the number of neighbors of the respective vertex.
    /// @param pvd vector of per-vertex data values, one value per mesh vertex. Must not include NAN values. See `smooth_pvd_nn_nan` if you need support for NAN values.
    /// @param num_iter number of iterations of smoothing to perform.
    /// @param with_nan whether you need support for NAN values in `pvd`. A bit slower if active. Ignored if `detectnan` is `true`.
    /// @param detect_nan whether to auto-detect presence of NAN values, ignoring the setting of `with_nan`.
    /// @return vector of smoothed per-vertex data values, same length as `pvd` param.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// std::vector<std::vector<size_t>> mesh_adj = surface.as_adjlist();
    /// std::vector<float> pvd = {1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7};
    /// std::vector<float> pvd_smooth = fs::Mesh::smooth_pvd_nn(mesh_adj, pvd, 2);
    /// @endcode
    static std::vector<float> smooth_pvd_nn(const std::vector<std::vector<size_t>> mesh_adj, const std::vector<float> pvd, const size_t num_iter=1, const bool with_nan=true, const bool detect_nan=true) {
      assert(pvd.size() == mesh_adj.size());
      bool final_with_nan = with_nan;
      if (detect_nan) {
        final_with_nan = false;
        for(size_t i=0; i<pvd.size(); i++) {
          if(std::isnan(pvd[i])) {
            final_with_nan = true;
            break;
          }
        }
      }
      if (final_with_nan) {
        return fs::Mesh::_smooth_pvd_nn_nan(mesh_adj, pvd, num_iter);
      }
      std::vector<float> current_pvd_source;
      std::vector<float> current_pvd_smoothed = std::vector<float>(pvd.size());

      float val_sum;
      size_t num_neigh;
      for(size_t i = 0; i < num_iter; i++) {
        if(i == 0) {
          current_pvd_source = pvd;
        } else {
          current_pvd_source = current_pvd_smoothed;
        }
        for(size_t v_idx = 0; v_idx < mesh_adj.size(); v_idx++) {
          num_neigh = mesh_adj[v_idx].size();
          val_sum = current_pvd_source[v_idx] / (num_neigh+1);
          for(size_t neigh_rel_idx = 0; neigh_rel_idx < num_neigh; neigh_rel_idx++) {
            val_sum += current_pvd_source[mesh_adj[v_idx][neigh_rel_idx]] / (num_neigh+1);
          }
          current_pvd_smoothed[v_idx] = val_sum;
        }
      }
      return current_pvd_smoothed;
    }

    /// @brief Smooth given per-vertex data including NAN values using nearest neighbor smoothing based on adjacency list mesh representation.
    /// @private
    /// @param mesh_adj the mesh, given as an adjacency list. The outer vector has size num_vertices, and the inner vectors sizes are the number of neighbors of the respective vertex.
    /// @param pvd vector of per-vertex data values, one value per mesh vertex. Must not include NAN values. See `smooth_pvd_nn_nan` if you need support for NAN values.
    /// @param num_iter number of iterations of smoothing to perform.
    /// @return vector of smoothed per-vertex data values, same length as `pvd` param.
    /// @note This function is private, users should call `fs::Mesh::smooth_pvd_nn` instead.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// std::vector<std::vector<size_t>> mesh_adj = surface.as_adjlist();
    /// std::vector<float> pvd = {1.0, 1.1, 1.2, NAN, 1.4, 1.5, 1.6, 1.7};
    /// std::vector<float> pvd_smooth = fs::Mesh::smooth_pvd_nn(mesh_adj, pvd, 2);
    /// @endcode
    static std::vector<float> _smooth_pvd_nn_nan(const std::vector<std::vector<size_t>> mesh_adj, const std::vector<float> pvd, const size_t num_iter=1) {
      std::vector<float> current_pvd_source;
      std::vector<float> current_pvd_smoothed = std::vector<float>(pvd.size());

      float val_sum;
      size_t num_neigh;
      size_t num_non_nan_values;
      float neigh_val;
      for(size_t i = 0; i < num_iter; i++) {

        if(i == 0) {
          current_pvd_source = pvd;
        } else {
          current_pvd_source = current_pvd_smoothed;
        }

        for(size_t v_idx = 0; v_idx < mesh_adj.size(); v_idx++) {
          if(std::isnan(current_pvd_source[v_idx])) {
            current_pvd_smoothed[v_idx] = NAN;
            continue;
          }
          val_sum = current_pvd_source[v_idx];
          num_non_nan_values = 1;  // If we get here, the source vertex value is not NAN.
          num_neigh = mesh_adj[v_idx].size();
          for(size_t neigh_rel_idx = 0; neigh_rel_idx < num_neigh; neigh_rel_idx++) {
            neigh_val = current_pvd_source[mesh_adj[v_idx][neigh_rel_idx]];
            if(std::isnan(neigh_val)) {
              continue;
            } else {
              val_sum += neigh_val;
              num_non_nan_values++;
            }
          }
          current_pvd_smoothed[v_idx] = val_sum / (float)num_non_nan_values;
        }
      }
      return current_pvd_smoothed;
    }

    /// @brief Extend mesh neighborhoods based on mesh adjacency representation.
    /// @details This function is mainly extended to extend a source neighborhood representation (typically the mesh's `k=1` neighborhood, i.e., the adjacency list of the mesh) to a higher `k`. In a `k=3` neighborhood, the neighorhood around a source vertex includes all vertices in edge distance up to 3 from the source vertex (but not the source vertex itself).
    /// @param mesh_adj The adjacency list representation of the underlying mesh, the outer vector must have size `N` for a mesh with `N` vertices.
    /// @param extend_by the number of edges to hop to extend the neighborhoods.
    /// @param mesh_adj_ext the starting neighborhoods to extend, same representation as `mesh_adj`. The outer vector must have size `N` or `0`. If passed as an empty vector, this will be ignored and a copy of the `mesh_adj` is used as the `start_neighborhoods`.
    /// @return extended neighborhoods
    static std::vector<std::vector<size_t>> extend_adj(const std::vector<std::vector<size_t>> mesh_adj, const size_t extend_by=1, std::vector<std::vector<size_t>> mesh_adj_ext=std::vector<std::vector<size_t>>()) {
      size_t num_vertices = mesh_adj.size();
      if(mesh_adj_ext.size() == 0) {
        mesh_adj_ext = mesh_adj;
      }
      std::vector<size_t> neighborhood;
      std::vector<size_t> ext_neighborhood;
      for(size_t ext_idx = 0; ext_idx < extend_by; ext_idx++) {
        for(size_t source_vert_idx = 0; source_vert_idx < num_vertices; source_vert_idx++) {
          neighborhood = mesh_adj_ext[source_vert_idx]; // copy needed so we do not modify during iteration.
          // Extension: add all neighbors in distance one for all vertices in the neighborhood.
          for(size_t neigh_vert_rel_idx = 0; neigh_vert_rel_idx < neighborhood.size(); neigh_vert_rel_idx++) {
            for(size_t canidate_rel_idx = 0; canidate_rel_idx < mesh_adj[neighborhood[neigh_vert_rel_idx]].size(); canidate_rel_idx++) {
              if(mesh_adj[neighborhood[neigh_vert_rel_idx]][canidate_rel_idx] != source_vert_idx) {
                mesh_adj_ext[source_vert_idx].push_back(mesh_adj[neighborhood[neigh_vert_rel_idx]][canidate_rel_idx]);
              }
            }
          }
          // We need to remove duplicates.
          std::sort(mesh_adj_ext[source_vert_idx].begin(), mesh_adj_ext[source_vert_idx].end());
          mesh_adj_ext[source_vert_idx].erase(std::unique(mesh_adj_ext[source_vert_idx].begin(), mesh_adj_ext[source_vert_idx].end()), mesh_adj_ext[source_vert_idx].end());
        }
      }
      return mesh_adj_ext;
    }


    /// @brief Export this mesh to a file in Wavefront OBJ format.
    /// @param filename path to the output file, will be overwritten if existing.
    /// @throws std::runtime_error if the target file cannot be opened.
    /// @see fs::Mesh::to_obj if you want the string representation (without writing it to a file).
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// const std::string out_path = fs::util::fullpath({"/tmp", "mesh.obj"});
    /// surface.to_obj_file(out_path);
    /// @endcode
    void to_obj_file(const std::string& filename) const {
      fs::util::str_to_file(filename, this->to_obj());
    }

    /// @brief Compute a new mesh that is a submesh of this mesh, based on a subset of the vertices of this mesh.
    /// @param old_vertex_indices vector of vertex indices of this mesh, which should be included in the submesh.
    /// @param mapdir_fulltosubmesh whether to return a map from the old (full mesh) to the new (submesh)  vertex indices (`true`), or the other way around (`false`, the default) as the first element of the returned pair.
    /// @return a pair of the vertex index map (direction 'fullmesh to submesh' by default, but see 'mapdir_fulltosubmesh' parameter) and the submesh.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface;
    /// fs::read_surf(&surface, "examples/read_surf/lh.white");
    /// fs::Label label;
    /// fs::read_label(&label, "examples/read_label/lh.cortex.label");
    /// std::pair <std::unordered_map<int32_t, int32_t>, fs::Mesh> result = surface.submesh_vertex(label.vertex);
    /// fs::Mesh patch = result.second;
    /// auto vertexindexmap_submesh2full = result.first; // or '<std::unordered_map<int32_t, int32_t>' instead of 'auto'.
    /// @endcode
    std::pair <std::unordered_map<int32_t, int32_t>, fs::Mesh> submesh_vertex(const std::vector<int32_t> &old_vertex_indices, const bool mapdir_fulltosubmesh = false) const {
      fs::Mesh submesh;
      std::vector<float> new_vertices;
      std::vector<int> new_faces;
      std::unordered_map<int32_t, int32_t> vertex_index_map_full2submesh;
      int32_t new_vertex_idx = 0;
      for(size_t i = 0; i < old_vertex_indices.size(); i++) {
        vertex_index_map_full2submesh[old_vertex_indices[i]] = new_vertex_idx;
        new_vertices.push_back(this->vertices[size_t(old_vertex_indices[i])*3]);
        new_vertices.push_back(this->vertices[size_t(old_vertex_indices[i])*3+1]);
        new_vertices.push_back(this->vertices[size_t(old_vertex_indices[i])*3+2]);
        new_vertex_idx++;
      }
      int face_v0;
      int face_v1;
      int face_v2;
      for(size_t i = 0; i < this->num_faces(); i++) {
        face_v0 = this->faces[i*3];
        face_v1 = this->faces[i*3+1];
        face_v2 = this->faces[i*3+2];
        if((vertex_index_map_full2submesh.find(face_v0) != vertex_index_map_full2submesh.end()) && (vertex_index_map_full2submesh.find(face_v1) != vertex_index_map_full2submesh.end()) && (vertex_index_map_full2submesh.find(face_v2) != vertex_index_map_full2submesh.end())) {
          new_faces.push_back(vertex_index_map_full2submesh[face_v0]);
          new_faces.push_back(vertex_index_map_full2submesh[face_v1]);
          new_faces.push_back(vertex_index_map_full2submesh[face_v2]);
        }
      }
      submesh.vertices = new_vertices;
      submesh.faces = new_faces;

      std::pair <std::unordered_map<int32_t, int32_t>, fs::Mesh> result;
      if(! mapdir_fulltosubmesh) {  // Compute the new2old (reverse) vertex index map:
        std::unordered_map<int32_t, int32_t> vertex_index_map_submesh2full;
        for (auto const& pair: vertex_index_map_full2submesh) {
          vertex_index_map_submesh2full[pair.second] = pair.first;
        }
        result = std::pair <std::unordered_map<int32_t, int32_t>, fs::Mesh>(vertex_index_map_submesh2full, submesh);
      } else {
        result = std::pair <std::unordered_map<int32_t, int32_t>, fs::Mesh>(vertex_index_map_full2submesh, submesh);
      }

      return result;
    }

    /// @brief Given per-vertex data for a submesh, add NAN values inbetween to restore the original mesh size.
    /// @param data_submesh vector of per-vertex data values, one value per mesh vertex of the submesh.
    /// @param submesh_to_orig_mapping map<int, int>, mapping vertex indices of the submesh to vertex indices of the original, full mesh.
    /// @param orig_mesh_num_vertices number of vertices of the original, full mesh.
    /// @see `fs::Mesh::submesh_vertex` for how to get the `submesh_to_orig_mapping` parameter.
    /// @return vector of per-vertex data values, one value per mesh vertex of the original mesh. Values for vertices that are not part of the submesh are set to NAN.
    static std::vector<float> curv_data_for_orig_mesh(const std::vector<float> data_submesh, const std::unordered_map<int32_t, int32_t> submesh_to_orig_mapping, const int32_t orig_mesh_num_vertices, const float fill_value=std::numeric_limits<float>::quiet_NaN()) {

      if(submesh_to_orig_mapping.size() != data_submesh.size()) {
        throw std::domain_error("The number of vertices of the submesh and the number of values in the submesh_to_orig_mapping do not match: got " + std::to_string(data_submesh.size()) + " and " + std::to_string(submesh_to_orig_mapping.size()) + ".");
      }

      std::vector<float> data_orig_mesh(orig_mesh_num_vertices, fill_value);
      for(size_t i=0; i<data_submesh.size(); i++) {
          auto got = submesh_to_orig_mapping.find(int(i));
          if (got != submesh_to_orig_mapping.end()) {
            data_orig_mesh[got->second] = data_submesh[i];
          }
      }
      return(data_orig_mesh);
    }



    /// @brief Read a brainmesh from a Wavefront object format stream.
    /// @details This only reads the geometry, optional format extensions like materials are ignored (but files including them should parse fine).
    /// @param mesh pointer to fs:Mesh instance to be filled.
    /// @param is stream holding a text representation of a mesh in Wavefront object format.
    /// @see There exists an overloaded version that reads from a file.
    /// @throws std::domain_error if the file format is invalid.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface;
    /// const std::string in_path = fs::util::fullpath({"/tmp", "mesh.obj"});
    /// fs::Mesh::from_obj(&surface, in_path);
    /// @endcode
    static void from_obj(Mesh* mesh, std::istream* is) {
      std::string line;
      int line_idx = -1;

      std::vector<float> vertices;
      std::vector<int> faces;
      size_t num_lines_ignored = 0; // Not comments, but custom extensions or material data lines which are ignored by libfs.

      while (std::getline(*is, line)) {
        line_idx += 1;
        std::istringstream iss(line);
        if(fs::util::starts_with(line, "#")) {
          continue; // skip comment.
        } else {
          if(fs::util::starts_with(line, "v ")) {
            std::string elem_type_identifier; float x, y, z;
            if (!(iss >> elem_type_identifier >> x >> y >> z)) {
              throw std::domain_error("Could not parse vertex line " + std::to_string(line_idx+1) + " of OBJ data, invalid format.\n");
            }
            assert(elem_type_identifier == "v");
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
          } else if(fs::util::starts_with(line, "f ")) {
            std::string elem_type_identifier, v0raw, v1raw, v2raw; int v0, v1, v2;
            if (!(iss >> elem_type_identifier >> v0raw >> v1raw >> v2raw)) {
              throw std::domain_error("Could not parse face line " + std::to_string(line_idx+1) + " of OBJ data, invalid format.\n");
            }
            assert(elem_type_identifier == "f");

            // The OBJ format allows to specifiy face indices with slashes to also set normal and material indices.
            // So instead of a line like 'f 22 34 45', we could get 'f 3/1 4/2 5/3' or 'f 6/4/1 3/5/3 7/6/5' or 'f 7//1 8//2 9//3'.
            // We need to extract the stuff before the first slash and interprete it as int to get the vertex index we are looking for.
            std::size_t found_v0 = v0raw.find("/");
            std::size_t found_v1 = v1raw.find("/");
            std::size_t found_v2 = v2raw.find("/");
            if (found_v0 != std::string::npos) {
              v0raw = v0raw.substr(0, found_v0);
            }
            if (found_v1 != std::string::npos) {
              v1raw = v1raw.substr(0, found_v1);
            }
            if (found_v2 != std::string::npos) {
              v2raw = v0raw.substr(0, found_v2);
            }
            v0 = std::stoi(v0raw);
            v1 = std::stoi(v1raw);
            v2 = std::stoi(v2raw);

            // The vertex indices in Wavefront OBJ files are 1-based, so we have to substract 1 here.
            faces.push_back(v0 - 1);
            faces.push_back(v1 - 1);
            faces.push_back(v2 - 1);

          } else {
            num_lines_ignored++;
            continue;
          }

        }
      }
      #ifdef LIBFS_DBG_INFO
      if(num_lines_ignored > 0) {
        std::cout << LIBFS_APPTAG << "Ignored " << num_lines_ignored << " lines in Wavefront OBJ format mesh file.\n";
      }
      #endif
      mesh->vertices = vertices;
      mesh->faces = faces;
    }


    /// @brief Read a brainmesh from a Wavefront object format mesh file.
    /// @details This only reads the geometry, optional format extensions like materials are ignored (but files including them should parse fine).
    /// @see There exists an overloaded version that reads from a stream.
    /// @param mesh pointer to fs:Mesh instance to be filled.
    /// @param filename path to input wavefront obj mesh to be read.
    /// @throws std::runtime_error if the file cannot be read.
    /// @throws std::domain_error if the file format is invalid.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface;
    /// fs::Mesh::from_obj(&surface, "mesh.obj");
    /// @endcode
    static void from_obj(Mesh* mesh, const std::string& filename) {
      #ifdef LIBFS_DBG_INFO
      std::cout << LIBFS_APPTAG << "Reading brain mesh from Wavefront object format file " << filename << ".\n";
      #endif
      std::ifstream input(filename, std::fstream::in);
      if(input.is_open()) {
        Mesh::from_obj(mesh, &input);
        input.close();
      } else {
        throw std::runtime_error("Could not open Wavefront object format mesh file '" + filename + "' for reading.\n");
      }
    }


    /// @brief Read a brainmesh from an Object File format (OFF) stream.
    /// @param mesh pointer to fs:Mesh instance to be filled.
    /// @param is An open std::istream or derived class stream from which to read the data, e.g., std::ifstream or std::istringstream.
    /// @param source_filename optional, used in error messages only. The source file name, if any.
    /// @see There exists an overloaded version that reads from a file.
    /// @throws std::domain_error if the file format is invalid.
    static void from_off(Mesh* mesh, std::istream* is, const std::string& source_filename="") {

      std::string msg_source_file_part = source_filename.empty() ? "" : "'" + source_filename + "'";

      std::string line;
      int line_idx = -1;
      int noncomment_line_idx = -1;

      std::vector<float> vertices;
      std::vector<int> faces;
      size_t num_vertices = 0;
      size_t num_faces = 0;
      size_t num_edges = 0;
      size_t num_verts_parsed = 0;
      size_t num_faces_parsed = 0;
      float x, y, z;    // vertex xyz coords
      //bool has_color;
      //int r, g, b, a;   // vertex colors
      int num_verts_this_face, v0, v1, v2;   // face, defined by number of vertices and vertex indices.

      while (std::getline(*is, line)) {
        line_idx++;
        std::istringstream iss(line);
        if(fs::util::starts_with(line, "#")) {
          continue; // skip comment.
        } else {
          noncomment_line_idx++;
          if(noncomment_line_idx == 0) {
            std::string off_header_magic;
            if (!(iss >> off_header_magic)) {
              throw std::domain_error("Could not parse first header line " + std::to_string(line_idx+1) + " of OFF data, invalid format.\n");
            }
            if(!(off_header_magic == "OFF" || off_header_magic == "COFF")) {
              throw std::domain_error("OFF magic string invalid, file " + msg_source_file_part + " not in OFF format.\n");
            }
            //has_color = off_header_magic == "COFF";
          } else if (noncomment_line_idx == 1) {
            if (!(iss >> num_vertices >> num_faces >> num_edges)) {
              throw std::domain_error("Could not parse element count header line " + std::to_string(line_idx+1) + " of OFF data " + msg_source_file_part + ", invalid format.\n");
            }
          } else {

            if(num_verts_parsed < num_vertices) {
              if (!(iss >> x >> y >> z)) {
                throw std::domain_error("Could not parse vertex coordinate line " + std::to_string(line_idx+1) + " of OFF data " + msg_source_file_part + ", invalid format.\n");
              }
              vertices.push_back(x);
              vertices.push_back(y);
              vertices.push_back(z);
              num_verts_parsed++;
            } else {
              if(num_faces_parsed < num_faces) {
                if (!(iss >> num_verts_this_face >> v0 >> v1 >> v2)) {
                  throw std::domain_error("Could not parse face line " + std::to_string(line_idx+1) + " of OFF data " + msg_source_file_part + ", invalid format.\n");
                }
                if(num_verts_this_face != 3) {
                  throw std::domain_error("At OFF data " + msg_source_file_part + " line " + std::to_string(line_idx+1) + ": only triangular meshes supported.\n");
                }
                faces.push_back(v0);
                faces.push_back(v1);
                faces.push_back(v2);
                num_faces_parsed++;
              }
            }
          }
        }
      }
      if(num_verts_parsed < num_vertices) {
        throw std::domain_error("Vertex count mismatch between OFF data " + msg_source_file_part + " header (" + std::to_string(num_vertices) + ") and data (" + std::to_string(num_verts_parsed) + ").\n");
      }
      if(num_faces_parsed < num_faces) {
        throw std::domain_error("Face count mismatch between OFF data " + msg_source_file_part + " header  (" + std::to_string(num_faces) + ") and data (" + std::to_string(num_faces_parsed) + ").\n");
      }
      mesh->vertices = vertices;
      mesh->faces = faces;
    }


    /// @brief Read a brainmesh from an OFF format mesh file.
    /// @see There exists an overloaded version that reads from a stream.
    /// @details The OFF is the Object File Format (file extension .off) is a simple text-based mesh file format. Not to be confused with the Wavefront Object format (.obj).
    /// @param mesh pointer to fs:Mesh instance to be filled.
    /// @param filename path to input wavefront obj mesh to be read.
    /// @throws std::runtime_error if the file cannot be read.
    /// @throws std::domain_error if the file format is invalid.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface;
    /// fs::Mesh::from_off(&surface, "mesh.off");
    /// @endcode
    static void from_off(Mesh* mesh, const std::string& filename) {
      #ifdef LIBFS_DBG_INFO
      std::cout << LIBFS_APPTAG << "Reading brain mesh from OFF format file " << filename << ".\n";
      #endif
      std::ifstream input(filename, std::fstream::in);
      if(input.is_open()) {
        Mesh::from_off(mesh, &input);
        input.close();
      } else {
        throw std::runtime_error("Could not open Object file format (OFF) mesh file '" + filename + "' for reading.\n");
      }
    }


    /// @brief Read a brainmesh from a Stanford PLY format stream.
    /// @param mesh pointer to fs:Mesh instance to be filled.
    /// @param is An open std::istream or derived class stream from which to read the data, e.g., std::ifstream or std::istringstream.
    /// @see There exists an overloaded version that reads from a file.
    /// @throws std::domain_error if the file format is invalid.
    static void from_ply(Mesh* mesh, std::istream* is) {
      std::string line;
      int line_idx = -1;
      int noncomment_line_idx = -1;

      std::vector<float> vertices;
      std::vector<int> faces;

      bool in_header = true; // current status
      int num_verts = -1;
      int num_faces = -1;
      while (std::getline(*is, line)) {
        line_idx += 1;
        std::istringstream iss(line);
        if(fs::util::starts_with(line, "comment")) {
          continue; // skip comment.
        } else {
          noncomment_line_idx++;
          if(in_header) {
            if(noncomment_line_idx == 0) {
              if(line != "ply") throw std::domain_error("Invalid PLY file");
            } else if(noncomment_line_idx == 1) {
              if(line != "format ascii 1.0") throw std::domain_error("Unsupported PLY file format, only format 'format ascii 1.0' is supported.");
            }

            if(line == "end_header") {
              in_header = false;
            } else if(fs::util::starts_with(line, "element vertex")) {
              std::string elem, elem_type_identifier;
              if (!(iss >> elem >> elem_type_identifier >> num_verts)) {
                throw std::domain_error("Could not parse element vertex line of PLY header, invalid format.\n");
              }
            } else if(fs::util::starts_with(line, "element face")) {
              std::string elem, elem_type_identifier;
              if (!(iss >> elem >> elem_type_identifier >> num_faces)) {
                throw std::domain_error("Could not parse element face line of PLY header, invalid format.\n");
              }
            } // Other properties like vertex colors and normals are ignored for now.

          } else {  // in data part.
            if(num_verts < 1 || num_faces < 1) {
              throw std::domain_error("Invalid PLY file: missing element count lines of header.");
            }
            // Read vertices
            if(vertices.size() < (size_t)num_verts * 3) {
              float x,y,z;
              if (!(iss >> x >> y >> z)) {
                throw std::domain_error("Could not parse vertex line " + std::to_string(line_idx) + " of PLY data, invalid format.\n");
              }
              vertices.push_back(x);
              vertices.push_back(y);
              vertices.push_back(z);
            } else {
              if(faces.size() < (size_t)num_faces * 3) {
                int verts_per_face, v0, v1, v2;
                if (!(iss >> verts_per_face >> v0 >> v1 >> v2)) {
                  throw std::domain_error("Could not parse face line " + std::to_string(line_idx) + " of PLY data, invalid format.\n");
                }
                if(verts_per_face != 3) {
                  throw std::domain_error("Only triangular meshes are supported: PLY faces lines must contain exactly 3 vertex indices.\n");
                }
                faces.push_back(v0);
                faces.push_back(v1);
                faces.push_back(v2);
              }
            }
          }
        }
      }
      if(vertices.size() != (size_t)num_verts * 3) {
        std::cerr << "PLY header mentions " << num_verts << " vertices, but found " << vertices.size() / 3 << ".\n";
      }
      if(faces.size() != (size_t)num_faces * 3) {
        std::cerr << "PLY header mentions " << num_faces << " faces, but found " << faces.size() / 3 << ".\n";
      }
      mesh->vertices = vertices;
      mesh->faces = faces;
    }

    /// @brief Read a brainmesh from a Stanford PLY format mesh file.
    /// @details The PLY format exists in text and binary forms, and the binary form can be little endian or big endian. This file reads the ASCII text format version.
    /// @param mesh pointer to fs:Mesh instance to be filled.
    /// @param filename path to input wavefront obj mesh to be read.
    /// @throws std::runtime_error if the file cannot be read.
    /// @throws std::domain_error if the file format is invalid.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface;
    /// fs::Mesh::from_ply(&surface, "mesh.ply");
    /// @endcode
    static void from_ply(Mesh* mesh, const std::string& filename) {
      #ifdef LIBFS_DBG_INFO
      std::cout << LIBFS_APPTAG << "Reading brain mesh from PLY format file " << filename << ".\n";
      #endif
      std::ifstream input(filename, std::fstream::in);
      if(input.is_open()) {
        Mesh::from_ply(mesh, &input);
        input.close();
      } else {
        throw std::runtime_error("Could not open Stanford PLY format mesh file '" + filename + "' for reading.\n");
      }
    }


    /// @brief Return the number of vertices in this mesh.
    /// @return the vertex count
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// size_t nv = surface.num_vertices();
    /// @endcode
    size_t num_vertices() const {
      return(this->vertices.size() / 3);
    }

    /// @brief Return the number of faces in this mesh.
    /// @return the face count
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// size_t nv = surface.num_faces();
    /// @endcode
    size_t num_faces() const {
      return(this->faces.size() / 3);
    }

    /// @brief Retrieve a vertex index of a face, treating the faces vector as an nx3 matrix.
    /// @param i the row index, valid values are 0..num_faces-1.
    /// @param j the column index, valid values are 0..2 (for the 3 vertices of a face).
    /// @throws std::range_error on invalid index
    /// @return vertex index of vertex `j` of face `i`
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// int first_face_third_vert = surface.fm_at(0, 2);
    /// @endcode
    const int32_t& fm_at(const size_t i, const size_t j) const {
      size_t idx = _vidx_2d(i, j, 3);
      if(idx > this->faces.size()-1) {
        throw std::range_error("Indices (" + std::to_string(i) + "," + std::to_string(j) + ") into Mesh.faces out of bounds. Hit " + std::to_string(idx) + " with max valid index " + std::to_string(this->faces.size()-1) + ".\n");
      }
      return(this->faces[idx]);
    }


    /// @brief Get all vertex indices of the face, given by its index.
    /// @param face the face index
    /// @returns vector of length 3, the vertex indices of the face.
    /// @throws std::range_error on invalid index
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// auto first_face_verts = surface.face_vertices(0);
    /// @endcode
    std::vector<int32_t> face_vertices(const size_t face) const {
      if(face > this->num_faces()-1) {
        throw std::range_error("Index " + std::to_string(face) + " into Mesh.faces out of bounds, max valid index is " + std::to_string(this->num_faces()-1) + ".\n");
      }
      std::vector<int32_t> fv(3);
      fv[0] = this->fm_at(face, 0);
      fv[1] = this->fm_at(face, 1);
      fv[2] = this->fm_at(face, 2);
      return(fv);
    }

    /// @brief Get all coordinates of the vertex, given by its index.
    /// @param vertex the vertex index
    /// @returns vector of length 3, the x,y,z coordinates of the vertex.
    /// @throws std::range_error on invalid index
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// auto coords = surface.vertex_coords(0);
    /// @endcode
    std::vector<float> vertex_coords(const size_t vertex) const {
      if(vertex > this->num_vertices()-1) {
        throw std::range_error("Index " + std::to_string(vertex) + " into Mesh.vertices out of bounds, max valid index is " + std::to_string(this->num_vertices()-1) + ".\n");
      }
      std::vector<float> vc(3);
      vc[0] = this->vm_at(vertex, 0);
      vc[1] = this->vm_at(vertex, 1);
      vc[2] = this->vm_at(vertex, 2);
      return(vc);
    }

    /// @brief Retrieve a single (x, y, or z) coordinate of a vertex, treating the vertices vector as an nx3 matrix.
    /// @param i the row index, valid values are 0..num_vertices.
    /// @param j the column index, valid values are 0..2 (for the x,y,z coordinates).
    /// @throws std::range_error on invalid index
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// float v5_x = surface.vm_at(5, 0);
    /// float v5_y = surface.vm_at(5, 1);
    /// float v5_z = surface.vm_at(5, 2);
    /// @endcode
    const float& vm_at(const size_t i, const size_t j) const {
      size_t idx = _vidx_2d(i, j, 3);
      if(idx > this->vertices.size()-1) {
        throw std::range_error("Indices (" + std::to_string(i) + "," + std::to_string(j) + ") into Mesh.vertices out of bounds. Hit " + std::to_string(idx) + " with max valid index " + std::to_string(this->vertices.size()-1) + ".\n");
      }
      return(this->vertices[idx]);
    }

    /// @brief Return string representing the mesh in PLY format. Overload that works without passing a color vector.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// std::string ply_rep = surface.to_ply();
    /// @endcode
    std::string to_ply() const {
      std::vector<uint8_t> empty_col;
      return(this->to_ply(empty_col));
    }

    /// @brief Return string representing the mesh in PLY format.
    /// @param col u_char vector of RGB color values, 3 per vertex. They must appear by vertex, i.e. in order v0_red, v0_green, v0_blue, v1_red, v1_green, v1_blue. Leave empty if you do not want colors.
    /// @throws std::invalid_argument if the number of vertex colors does not match the number of vertices.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// std::string ply_rep = surface.to_ply();
    /// @endcode
    std::string to_ply(const std::vector<uint8_t> col) const {
      bool use_vertex_colors = col.size() != 0;
      std::stringstream plys;
      plys << "ply\nformat ascii 1.0\n";
      plys << "element vertex " << this->num_vertices() << "\n";
      plys << "property float x\nproperty float y\nproperty float z\n";
      if(use_vertex_colors) {
        if(col.size() != this->vertices.size()) {
          throw std::invalid_argument("Number of vertex coordinates and vertex colors must match when writing PLY file, but got " + std::to_string(this->vertices.size()) + " and " + std::to_string(col.size()) + ".");
        }
        plys << "property uchar red\nproperty uchar green\nproperty uchar blue\n";
      }
      plys << "element face " << this->num_faces() << "\n";
      plys << "property list uchar int vertex_index\n";
      plys << "end_header\n";

      #ifdef LIBFS_DBG_DEBUG
      fs::util::log("Writing " + std::to_string(this->vertices.size()/3) + " PLY format vertices.", "INFO");
      #endif

      for(size_t vidx=0; vidx<this->vertices.size();vidx+=3) {  // vertex coords
        plys << vertices[vidx] << " " << vertices[vidx+1] << " " << vertices[vidx+2];
        if(use_vertex_colors) {
          plys << " " << (int)col[vidx] << " " << (int)col[vidx+1] << " " << (int)col[vidx+2];
        }
        plys << "\n";
      }

      #ifdef LIBFS_DBG_DEBUG
      fs::util::log("Writing " + std::to_string(this->faces.size()/3) + " PLY format faces.", "INFO");
      #endif

      const int num_vertices_per_face = 3;
      for(size_t fidx=0; fidx<this->faces.size();fidx+=3) { // faces: vertex indices, 0-based
        plys << num_vertices_per_face << " " << faces[fidx] << " " << faces[fidx+1] << " " << faces[fidx+2] << "\n";
      }
      return(plys.str());
    }

    /// @brief Export this mesh to a file in Stanford PLY format.
    /// @throws std::runtime_error if the target file cannot be opened.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// surface.to_ply_file("mesh.ply");
    /// @endcode
    void to_ply_file(const std::string& filename) const {
      #ifdef LIBFS_DBG_INFO
      fs::util::log("Writing mesh to PLY file '" + filename + "'.", "INFO");
      #endif
      fs::util::str_to_file(filename, this->to_ply());
    }

    /// @brief Export this mesh to a file in Stanford PLY format with vertex colors.
    /// @throws std::runtime_error if the target file cannot be opened, std::invalid_argument if the number of vertex colors does not match the number of vertices.
    void to_ply_file(const std::string& filename, const std::vector<uint8_t> col) const {
      fs::util::str_to_file(filename, this->to_ply(col));
    }

    /// @brief Return string representing the mesh in OFF format. Overload that works without passing a color vector.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// std::string off_rep = surface.to_off();
    /// @endcode
    std::string to_off() const {
      std::vector<uint8_t> empty_col;
      return(this->to_off(empty_col));
    }

    /// @brief Return string representing the mesh in PLY format.
    /// @param col u_char vector of RGB color values, 3 per vertex. They must appear by vertex, i.e. in order v0_red, v0_green, v0_blue, v1_red, v1_green, v1_blue. Leave empty if you do not want colors.
    /// @throws std::invalid_argument if the number of vertex colors does not match the number of vertices.
    std::string to_off(const std::vector<uint8_t> col) const {
      bool use_vertex_colors = col.size() != 0;
      std::stringstream offs;
      if(use_vertex_colors) {
        #ifdef LIBFS_DBG_INFO
        fs::util::log("Writing OFF representation of mesh with vertex colors.", "INFO");
        #endif
        if(col.size() != this->vertices.size()) {
          throw std::invalid_argument("Number of vertex coordinates and vertex colors must match when writing OFF file but got " + std::to_string(this->vertices.size()) + " and " + std::to_string(col.size()) + ".");
        }
        offs << "COFF\n";
      } else {
        #ifdef LIBFS_DBG_INFO
        fs::util::log("Writing OFF representation of mesh without vertex colors.", "INFO");
        #endif
        offs << "OFF\n";
      }
      offs << this->num_vertices() << " " << this->num_faces() << " 0\n";

      for(size_t vidx=0; vidx<this->vertices.size();vidx+=3) {  // vertex coords
        offs << vertices[vidx] << " " << vertices[vidx+1] << " " << vertices[vidx+2];
        if(use_vertex_colors) {
          offs << " " << (int)col[vidx] << " " << (int)col[vidx+1] << " " << (int)col[vidx+2] << " 255";
        }
        offs << "\n";
      }

      const int num_vertices_per_face = 3;
      for(size_t fidx=0; fidx<this->faces.size();fidx+=3) { // faces: vertex indices, 0-based
        offs << num_vertices_per_face << " " << faces[fidx] << " " << faces[fidx+1] << " " << faces[fidx+2] << "\n";
      }
      return(offs.str());
    }

    /// @brief Export this mesh to a file in OFF format.
    /// @throws std::runtime_error if the target file cannot be opened.
    ///
    /// #### Examples
    ///
    /// @code
    /// fs::Mesh surface = fs::Mesh::construct_cube();
    /// surface.to_off_file("mesh.off");
    /// @endcode
    void to_off_file(const std::string& filename) const {
      fs::util::str_to_file(filename, this->to_off());
    }

    /// @brief Export this mesh to a file in OFF format with vertex colors (COFF).
    /// @throws std::runtime_error if the target file cannot be opened, std::invalid_argument if the number of vertex colors does not match the number of vertices.
    void to_off_file(const std::string& filename, const std::vector<uint8_t> col) const {
      fs::util::str_to_file(filename, this->to_off(col));
    }
  };


  /// Models a FreeSurfer curv file that contains per-vertex float data.
  struct Curv {

    /// Construct a Curv instance from the given per-vertex data.
    Curv(std::vector<float> curv_data) :
      num_faces(100000), num_vertices(0), num_values_per_vertex(1) { data = curv_data; num_vertices = int(data.size()); }

    /// Construct an empty Curv instance.
    Curv() :
      num_faces(100000), num_vertices(0), num_values_per_vertex(1) {}

    /// The number of faces of the mesh to which this belongs, typically irrelevant and ignored.
    int32_t num_faces;

    /// The curvature data, one value per vertex. Something like the cortical thickness at each vertex.
    std::vector<float> data;

    /// The number of vertices of the mesh to which this belongs. Can be deduced from length of 'data'.
    int32_t num_vertices;

    /// The number of values per vertex, stored in this file. Almost all apps (including FreeSurfer itself) only support a value of 1 here. Ignored by most apps, and assumed to be 1.
    int32_t num_values_per_vertex;
  };

  /// The colortable from an Annot file, can be used for parcellations and integer labels. Typically each index (in all fields) describes a brain region.
  struct Colortable {
    std::vector<int32_t> id;  ///< internal region index
    std::vector<std::string> name;   ///< region name
    std::vector<int32_t> r;   ///< red channel of RGBA color
    std::vector<int32_t> g;   ///< blue channel of RGBA color
    std::vector<int32_t> b;   ///< green channel of RGBA color
    std::vector<int32_t> a;   ///< alpha channel of RGBA color
    std::vector<int32_t> label;   ///< label integer computed from rgba values. Maps to the Annot.vertex_label field.

    /// @brief Get the number of enties (regions) in this Colortable.
    size_t num_entries() const {
      size_t num_ids = this->id.size();
      if(this->name.size() != num_ids || this->r.size() != num_ids || this->g.size() != num_ids || this->b.size() != num_ids || this->a.size() != num_ids || this->label.size() != num_ids) {
        std::cerr << "Inconsistent Colortable, vector sizes do not match.\n";
      }
      return num_ids;
    }

    /// @brief Get the index of a region in the Colortable by region name. Returns a negative value if the region is not found.
    int32_t get_region_idx(const std::string& query_name) const {
      for(size_t i = 0; i<this->num_entries(); i++) {
        if(this->name[i] == query_name) {
          return (int32_t)i;
        }
      }
      return(-1);
    }

    /// @brief Get the index of a region in the Colortable by label. Returns a negative value if the region is not found.
    int32_t get_region_idx(int32_t query_label) const {
      for(size_t i = 0; i<this->num_entries(); i++) {
        if(this->label[i] == query_label) {
          return (int32_t)i;
        }
      }
      return(-1);
    }

  };


  /// An annotation, also known as a brain surface parcellation. Assigns to each vertex a region, identified by the region_label. The region name and color for each region can be found in the Colortable.
  struct Annot {
    std::vector<int32_t> vertex_indices;  ///< Indices of the vertices, these always go from 0 to N-1 (where N is the number of vertices in the respective surface/annotation). Not really needed.
    std::vector<int32_t> vertex_labels;   ///< The label code for each vertex, defining the region it belongs to. Check in the Colortable for a region that has this label.
    Colortable colortable;  ///< A Colortable defining the regions (most importantly, the region name and visualization color).

    /// @brief Get all vertices of a region given by name in the brain surface parcellation. Returns an integer vector, the vertex indices.
    std::vector<int32_t> region_vertices(const std::string& region_name) const {
      int32_t region_idx = this->colortable.get_region_idx(region_name);
      if(region_idx >= 0) {
        return(this->region_vertices(this->colortable.label[region_idx]));
      } else {
        std::cerr << "No such region in annot, returning empty vector.\n";
        std::vector<int32_t> empty;
        return(empty);
      }
    }

    /// @brief Get all vertices of a region given by label in the brain surface parcellation. Returns an integer vector, the vertex indices.
    std::vector<int32_t> region_vertices(int32_t region_label) const {
      std::vector<int32_t> reg_verts;
      for(size_t i=0; i<this->vertex_labels.size(); i++) {
        if(this->vertex_labels[i] == region_label) {
          reg_verts.push_back(int(i));
        }
      }
      return(reg_verts);
    }

    /// @brief Get the vertex colors as an array of uchar values, 3 consecutive values are the red, green and blue channel values for a single vertex.
    /// @param alpha whether to include the alpha channel and return 4 values per vertex instead of 3.
    std::vector<uint8_t> vertex_colors(bool alpha = false) const {
      int num_channels = alpha ? 4 : 3;
      std::vector<uint8_t> col;
      col.reserve(this->num_vertices() * num_channels);
      std::vector<size_t> vertex_region_indices = this->vertex_regions();
      for(size_t i=0; i<this->num_vertices(); i++) {
        col.push_back(this->colortable.r[vertex_region_indices[i]]);
        col.push_back(this->colortable.g[vertex_region_indices[i]]);
        col.push_back(this->colortable.b[vertex_region_indices[i]]);
        if(alpha) {
          col.push_back(this->colortable.a[vertex_region_indices[i]]);
        }
      }
      return(col);
    }

    /// @brief Get the number of vertices of this parcellation (or the associated surface).
    /// @throws std::runtime_error on invalid annot
    size_t num_vertices() const {
      size_t nv = this->vertex_indices.size();
      if(this->vertex_labels.size() != nv) {
        throw std::runtime_error("Inconsistent annot, number of vertex indices and labels does not match.\n");
      }
      return nv;
    }

    /// @brief Compute the region indices in the Colortable for all vertices in this brain surface parcellation. With the region indices, it becomes very easy to obtain all region names, labels, and color channel values from the Colortable.
    /// @see The function `vertex_region_names` uses this function to get the region names for all vertices.
    std::vector<size_t> vertex_regions() const {
      std::vector<size_t> vert_reg;
      for(size_t i=0; i<this->num_vertices(); i++) {
        vert_reg.push_back(0);  // init with zeros.
      }
      for(size_t region_idx=0; region_idx<this->colortable.num_entries(); region_idx++) {
        std::vector<int32_t> reg_vertices = this->region_vertices(this->colortable.label[region_idx]);
        for(size_t region_vert_local_idx=0;  region_vert_local_idx<reg_vertices.size(); region_vert_local_idx++) {
          int32_t region_vert_idx = reg_vertices[region_vert_local_idx];
          vert_reg[region_vert_idx] = region_idx;
        }
      }
      return vert_reg;
    }

    /// @brief Compute the region names in the Colortable for all vertices in this brain surface parcellation.
    std::vector<std::string> vertex_region_names() const {
      std::vector<std::string> region_names;
      std::vector<size_t> vertex_region_indices = this->vertex_regions();
      for(size_t i=0; i<this->num_vertices(); i++) {
        region_names.push_back(this->colortable.name[vertex_region_indices[i]]);
      }
      return(region_names);
    }
  };


  /// Models the header of an MGH file.
  struct MghHeader {
    MghHeader() {}  ///< Empty default constuctor.
    MghHeader(Curv curv) {  ///< Constuctor to fill header from a Curv instance.
      dim1length = curv.data.size();
      dim2length = 1;
      dim3length = 1;
      dim4length = 1;
      dtype = fs::MRI_FLOAT;
    }
    MghHeader(std::vector<float> curv_data) { ///< Constuctor to fill header from a 1D float array (curv data).
      dim1length = curv_data.size();
      dim2length = 1;
      dim3length = 1;
      dim4length = 1;
      dtype = fs::MRI_FLOAT;
    }
    int32_t dim1length = 0;  ///< size of data along 1st dimension
    int32_t dim2length = 0;  ///< size of data along 2nd dimension
    int32_t dim3length = 0;  ///< size of data along 3rd dimension
    int32_t dim4length = 0;  ///< size of data along 4th dimension

    int32_t dtype = 0;  ///< the MRI data type
    int32_t dof = 0;  ///< typically ignored
    int16_t ras_good_flag = 0; ///< flag indicating whether the data in the RAS fields (Mdc, Pxyz_c) are valid. 1 means valid, everything else means invalid.

    /// @brief Compute the number of values based on the dim*length header fields.
    size_t num_values() const {
      return((size_t) dim1length * dim2length * dim3length * dim4length);
    }

    float xsize = 0.0;  ///< size of voxels along 1st axis (x or r)
    float ysize = 0.0;  ///< size of voxels along 2nd axis (y or a)
    float zsize = 0.0;  ///< size of voxels along 3rd axis (z or s)
    std::vector<float> Mdc;  ///< matrix
    std::vector<float> Pxyz_c;  ///< x,y,z coordinates of central vertex
  };

  /// Models the data of an MGH file. Currently these are 1D vectors, but one can compute the 4D array using the dimXlength fields of the respective MghHeader.
  struct MghData {
    MghData() {}
    MghData(std::vector<int32_t> curv_data) { data_mri_int = curv_data; }  ///< constructor to create MghData from MRI_INT (int32_t) data.
    explicit MghData(std::vector<uint8_t> curv_data) { data_mri_uchar = curv_data; }  ///< constructor to create MghData from MRI_UCHAR (uint8_t) data.
    explicit MghData(std::vector<short> curv_data) { data_mri_short = curv_data; }  ///< constructor to create MghData from MRI_SHORT (short) data.
    MghData(std::vector<float> curv_data) { data_mri_float = curv_data; }  ///< constructor to create MghData from MRI_FLOAT (float) data.
    MghData(Curv curv) { data_mri_float = curv.data; }  ///< constructor to create MghData from a Curv instance
    std::vector<int32_t> data_mri_int;  ///< data of type MRI_INT, check the dtype to see whether this is relevant for this instance.
    std::vector<uint8_t> data_mri_uchar;  ///< data of type MRI_UCHAR, check the dtype to see whether this is relevant for this instance.
    std::vector<float> data_mri_float;  ///< data of type MRI_FLOAT, check the dtype to see whether this is relevant for this instance.
    std::vector<short> data_mri_short;  ///< data of type MRI_SHORT, check the dtype to see whether this is relevant for this instance.
  };

  /// Models a whole MGH file.
  struct Mgh {
    MghHeader header;  ///< Header for this MGH instance.
    MghData data;  ///< 4D data for this MGH instance.
    Mgh() {}  ///< Empty default constuctor.
    Mgh(Curv curv) {  ///< Constuctor to create MGH instance from Curv instance.
      header = MghHeader(curv);
      data = MghData(curv);
    }
    Mgh(std::vector<float> curv_data) {  ///< Constuctor to create MGH instance from a 1D float array (curv data).
      header = MghHeader(curv_data);
      data = MghData(curv_data);
    }
  };

  /// @brief A simple 4D array datastructure, useful for representing volume data.
  /// @details By convention, for FreeSurfer data, the order of the 4 dimensions is: *time*, *x*, *y*, *z*.
  template<class T>
  struct Array4D {
    /// Constructor for creating an empty 4D array of the given dimensions.
    Array4D(unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4) :
      d1(d1), d2(d2), d3(d3), d4(d4), data(d1*d2*d3*d4) {}

    /// Constructor for creating an empty 4D array based on dimensions specified in an fs::MghHeader.
    Array4D(MghHeader *mgh_header) :
      d1(mgh_header->dim1length), d2(mgh_header->dim2length), d3(mgh_header->dim3length), d4(mgh_header->dim4length), data(d1*d2*d3*d4) {}

    /// Constructor for creating an empty 4D array based on dimensions specified in the header of an fs::Mgh. Does not init the data.
    Array4D(Mgh *mgh) : // This does NOT init the data atm.
      d1(mgh->header.dim1length), d2(mgh->header.dim2length), d3(mgh->header.dim3length), d4(mgh->header.dim4length), data(d1*d2*d3*d4) {}

    /// Get the value at the given 4D position.
    const T& at(const unsigned int i1, const unsigned int i2, const unsigned int i3, const unsigned int i4) const {
      return data[get_index(i1, i2, i3, i4)];
    }

    /// Get the index in the vector for the given 4D position.
    unsigned int get_index(const unsigned int i1, const unsigned int i2, const unsigned int i3, const unsigned int i4) const {
      assert(i1 >= 0 && i1 < d1);
      assert(i2 >= 0 && i2 < d2);
      assert(i3 >= 0 && i3 < d3);
      assert(i4 >= 0 && i4 < d4);
      return (((i1*d2 + i2)*d3 + i3)*d4 + i4);
    }

    /// Get number of values/voxels.
    unsigned int num_values() const {
      return(d1*d2*d3*d4);
    }

    unsigned int d1;  ///< size of data along 1st dimension
    unsigned int d2;  ///< size of data along 2nd dimension
    unsigned int d3;  ///< size of data along 3rd dimension
    unsigned int d4;  ///< size of data along 4th dimension
    std::vector<T> data;  ///< the data, as a 1D vector. Use fs::Array4D::at for easy access in 4D.
  };

  // More declarations, should also go to separate header.
  void read_mgh_header(MghHeader*, const std::string&);
  void read_mgh_header(MghHeader*, std::istream*);
  template <typename T> std::vector<T> _read_mgh_data(MghHeader*, const std::string&);
  template <typename T> std::vector<T> _read_mgh_data(MghHeader*, std::istream*);
  std::vector<int32_t> _read_mgh_data_int(MghHeader*, const std::string&);
  std::vector<int32_t> _read_mgh_data_int(MghHeader*, std::istream*);
  std::vector<uint8_t> _read_mgh_data_uchar(MghHeader*, const std::string&);
  std::vector<uint8_t> _read_mgh_data_uchar(MghHeader*, std::istream*);
  std::vector<short> _read_mgh_data_short(MghHeader*, const std::string&);
  std::vector<short> _read_mgh_data_short(MghHeader*, std::istream*);
  std::vector<float> _read_mgh_data_float(MghHeader*, const std::string&);
  std::vector<float> _read_mgh_data_float(MghHeader*, std::istream*);


  /// @brief Read a FreeSurfer volume file in MGH format into the given Mgh struct.
  /// @param mgh An Mgh instance that should be filled with the data from the filename.
  /// @param filename Path to the input MGH file.
  /// @see There exists an overloaded version that reads from a stream.
  /// @throws runtime_error if the file uses an unsupported MRI data type.
  ///
  /// #### Examples
  ///
  /// @code
  /// fs::Mgh mgh;
  /// fs::read_mgh(&mgh, "somebrain.mgh");
  /// @endcode
  void read_mgh(Mgh* mgh, const std::string& filename) {
    MghHeader mgh_header;
    read_mgh_header(&mgh_header, filename);
    mgh->header = mgh_header;
    if(mgh->header.dtype == MRI_INT) {
      std::vector<int32_t> data = _read_mgh_data_int(&mgh_header, filename);
      mgh->data.data_mri_int = data;
    } else if(mgh->header.dtype == MRI_UCHAR) {
      std::vector<uint8_t> data = _read_mgh_data_uchar(&mgh_header, filename);
      mgh->data.data_mri_uchar = data;
    } else if(mgh->header.dtype == MRI_FLOAT) {
      std::vector<float> data = _read_mgh_data_float(&mgh_header, filename);
      mgh->data.data_mri_float = data;
    } else if(mgh->header.dtype == MRI_SHORT) {
      std::vector<short> data = _read_mgh_data_short(&mgh_header, filename);
      mgh->data.data_mri_short = data;
    } else {
      #ifdef LIBFS_DBG_INFO
      if(fs::util::ends_with(filename, ".mgz")) {
        std::cout << LIBFS_APPTAG << "Note: your MGH filename ends with '.mgz'. Keep in mind that MGZ format is not supported directly. You can ignore this message if you wrapped a gz stream.\n";
      }
      #endif
      throw std::runtime_error("Not reading MGH data from file '" + filename + "', data type " + std::to_string(mgh->header.dtype) + " not supported yet.\n");
    }
  }

  /// @brief Read a vector of subject identifiers from a FreeSurfer subjects file.
  /// @param filename a text file that contains one subject identifier per line.
  /// @throws runtime_error if the file cannot be read
  ///
  /// #### Examples
  ///
  /// @code
  /// std::vector<std::string> subjects = fs::read_subjectsfile("subjects.txt");
  /// @endcode
  std::vector<std::string> read_subjectsfile(const std::string& filename) {
    std::vector<std::string> subjects;
    std::ifstream input(filename, std::fstream::in);
    std::string line;

    if(! input.is_open()) {
      throw std::runtime_error("Could not open subjects file '" + filename + "'.\n");
    }

    while( std::getline( input, line ) ) {
        subjects.push_back(line);
    }
    return(subjects);
  }

  /// @brief Read MGH data from a stream.
  /// @param mgh An Mgh instance that should be filled with the data from the stream.
  /// @param is Pointer to an open istream from which to read the MGH data.
  /// @see There exists an overloaded version that reads from a file.
  /// @throws runtime_error if the file uses an unsupported MRI data type.
  void read_mgh(Mgh* mgh, std::istream* is) {
    MghHeader mgh_header;
    read_mgh_header(&mgh_header, is);
    mgh->header = mgh_header;
    if(mgh->header.dtype == MRI_INT) {
      std::vector<int32_t> data = _read_mgh_data_int(&mgh_header, is);
      mgh->data.data_mri_int = data;
    } else if(mgh->header.dtype == MRI_UCHAR) {
      std::vector<uint8_t> data = _read_mgh_data_uchar(&mgh_header, is);
      mgh->data.data_mri_uchar = data;
    } else if(mgh->header.dtype == MRI_FLOAT) {
      std::vector<float> data = _read_mgh_data_float(&mgh_header, is);
      mgh->data.data_mri_float = data;
    } else if(mgh->header.dtype == MRI_SHORT) {
      std::vector<short> data = _read_mgh_data_short(&mgh_header, is);
      mgh->data.data_mri_short = data;
    } else {
      throw std::runtime_error("Not reading data from MGH stream, data type " + std::to_string(mgh->header.dtype) + " not supported yet.\n");
    }
  }

  /// @brief Read an MGH header from a stream.
  /// @param mgh_header An MghHeader instance that should be filled with the data from the stream.
  /// @param is Pointer to an open istream from which to read the MGH data.
  /// @see There exists an overloaded version that reads from a file.
  /// @throws runtime_error if the file uses an unsupported MRI file format version. Only version 1 is supported (the only existing version to my knowledge).
  void read_mgh_header(MghHeader* mgh_header, std::istream* is) {
    const int MGH_VERSION = 1;

    int format_version = _freadt<int32_t>(*is);
    if(format_version != MGH_VERSION) {
      throw std::runtime_error("Invalid MGH file or unsupported file format version: expected version " + std::to_string(MGH_VERSION) + ", found " + std::to_string(format_version) + ".\n");
    }
    mgh_header->dim1length =  _freadt<int32_t>(*is);
    mgh_header->dim2length =  _freadt<int32_t>(*is);
    mgh_header->dim3length =  _freadt<int32_t>(*is);
    mgh_header->dim4length =  _freadt<int32_t>(*is);

    mgh_header->dtype =  _freadt<int32_t>(*is);
    mgh_header->dof =  _freadt<int32_t>(*is);

    int unused_header_space_size_left = 256;  // in bytes
    mgh_header->ras_good_flag =  _freadt<int16_t>(*is);
    unused_header_space_size_left -= 2; // for the ras_good_flag

    // Read the RAS part of the header.
    if(mgh_header->ras_good_flag == 1) {
      mgh_header->xsize =  _freadt<float>(*is);
      mgh_header->ysize =  _freadt<float>(*is);
      mgh_header->zsize =  _freadt<float>(*is);

      for(int i=0; i<9; i++) {
        mgh_header->Mdc.push_back( _freadt<float>(*is));
      }
      for(int i=0; i<3; i++) {
        mgh_header->Pxyz_c.push_back( _freadt<float>(*is));
      }
      unused_header_space_size_left -= 60;
    }

    // Advance to data part. We do not seek here because that is not
    // possible if the stream is gzip-wrapped with zstr, as in the read_mgz example.
    uint8_t discarded;
    while(unused_header_space_size_left > 0) {
      discarded = _freadt<uint8_t>(*is);
      unused_header_space_size_left -= 1;
    }
    (void)discarded; // Suppress warnings about unused variable.
  }

  /// @brief Read MRI_INT data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  std::vector<int32_t> _read_mgh_data_int(MghHeader* mgh_header, const std::string& filename) {
    if(mgh_header->dtype != MRI_INT) {
      std::cerr << "Expected MRI data type " << MRI_INT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<int32_t>(mgh_header, filename));
  }

  /// @brief Read MRI_INT data from a stream.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  std::vector<int32_t> _read_mgh_data_int(MghHeader* mgh_header, std::istream* is) {
    if(mgh_header->dtype != MRI_INT) {
      std::cerr << "Expected MRI data type " << MRI_INT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<int32_t>(mgh_header, is));
  }

  /// @brief Read MRI_SHORT data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  std::vector<short> _read_mgh_data_short(MghHeader* mgh_header, const std::string& filename) {
    if(mgh_header->dtype != MRI_SHORT) {
      std::cerr << "Expected MRI data type " << MRI_SHORT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<short>(mgh_header, filename));
  }

  /// @brief Read MRI_SHORT data from a stream.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  std::vector<short> _read_mgh_data_short(MghHeader* mgh_header, std::istream* is) {
    if(mgh_header->dtype != MRI_SHORT) {
      std::cerr << "Expected MRI data type " << MRI_SHORT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<short>(mgh_header, is));
  }


  /// @brief Read the header of a FreeSurfer volume file in MGH format into the given MghHeader struct.
  ///
  /// @param mgh_header An MghHeader instance that should be filled with the data from the file.
  /// @param filename Path to the file from which to read the MGH data.
  /// @see There exists an overloaded version that reads from a stream.
  /// @throws runtime_error if the file cannot be opened
  void read_mgh_header(MghHeader* mgh_header, const std::string& filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ios_base::in | std::ios::binary);
    if(ifs.is_open()) {
      read_mgh_header(mgh_header, &ifs);
      ifs.close();
    } else {
      throw std::runtime_error("Unable to open MGH file '" + filename + "'.\n");
    }
  }


  /// @brief Read arbitrary MGH data from a file.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @throws runtime_error if the file cannot be opened
  /// @private
  template <typename T>
  std::vector<T> _read_mgh_data(MghHeader* mgh_header, const std::string& filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ios_base::in | std::ios::binary);
    if(ifs.is_open()) {
      ifs.seekg(284, ifs.beg); // skip to end of header and beginning of data

      int num_values = int(mgh_header->num_values());
      std::vector<T> data;
      for(int i=0; i<num_values; i++) {
        data.push_back( _freadt<T>(ifs));
      }
      ifs.close();
      return(data);
    } else {
      throw std::runtime_error("Unable to open MGH file '" + filename + "'.\n");
    }
  }


  /// @brief Read arbitrary MGH data from a stream. The stream must be open and at the beginning of the MGH data.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  template <typename T>
  std::vector<T> _read_mgh_data(MghHeader* mgh_header, std::istream* is) {
    int num_values = int(mgh_header->num_values());
    std::vector<T> data;
    for(int i=0; i<num_values; i++) {
      data.push_back( _freadt<T>(*is));
    }
    return(data);
  }


  /// @brief Read MRI_FLOAT data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  std::vector<float> _read_mgh_data_float(MghHeader* mgh_header, const std::string& filename) {
    if(mgh_header->dtype != MRI_FLOAT) {
      std::cerr << "Expected MRI data type " << MRI_FLOAT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<float>(mgh_header, filename));
  }

  /// @brief Read MRI_FLOAT data from an MGH stream
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  std::vector<float> _read_mgh_data_float(MghHeader* mgh_header, std::istream* is) {
    if(mgh_header->dtype != MRI_FLOAT) {
      std::cerr << "Expected MRI data type " << MRI_FLOAT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<float>(mgh_header, is));
  }

  /// @brief Read MRI_UCHAR data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  std::vector<uint8_t> _read_mgh_data_uchar(MghHeader* mgh_header, const std::string& filename) {
    if(mgh_header->dtype != MRI_UCHAR) {
      std::cerr << "Expected MRI data type " << MRI_UCHAR << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<uint8_t>(mgh_header, filename));
  }

  /// @brief Read MRI_UCHAR data from an MGH stream
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  std::vector<uint8_t> _read_mgh_data_uchar(MghHeader* mgh_header, std::istream* is) {
    if(mgh_header->dtype != MRI_UCHAR) {
      std::cerr << "Expected MRI data type " << MRI_UCHAR << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<uint8_t>(mgh_header, is));
  }

  /// @brief Read a brain mesh from a file in binary FreeSurfer 'surf' format into the given Mesh instance.
  ///
  /// @param surface a Mesh instance representing a vertex-indexed tri-mesh. This will be filled.
  /// @param filename The path to the file from which to read the mesh. Must be in binary FreeSurfer surf format. An example file is `surf/lh.white`.
  /// @throws runtime_error if the file cannot be opened, domain_error if the surf file magic mismatches.
  /// @see fs::read_mesh, a generalized version that supports other mesh file formats as well.
  ///
  /// #### Examples
  ///
  /// @code
  /// fs::Mesh surface;
  /// fs::read_surf(&surface, "lh.white");
  /// @endcode
  void read_surf(Mesh* surface, const std::string& filename) {
    const int SURF_TRIS_MAGIC = 16777214;
    std::ifstream is;
    is.open(filename, std::ios_base::in | std::ios::binary);
    if(is.is_open()) {
      int magic = _fread3(is);
      if(magic != SURF_TRIS_MAGIC) {
        throw std::domain_error("Surf file '" + filename + "' magic code in header did not match: expected " + std::to_string(SURF_TRIS_MAGIC) + ", found " + std::to_string(magic) + ".\n");
      }
      std::string created_line = _freadstringnewline(is);
      std::string comment_line = _freadstringnewline(is);
      int num_verts =  _freadt<int32_t>(is);
      int num_faces =  _freadt<int32_t>(is);
      #ifdef LIBFS_DBG_INFO
      std::cout << LIBFS_APPTAG << "Read surface file with " << num_verts << " vertices, " << num_faces << " faces.\n";
      #endif
      std::vector<float> vdata;
      for(int i=0; i<(num_verts*3); i++) {
        vdata.push_back( _freadt<float>(is));
      }
      std::vector<int> fdata;
      for(int i=0; i<(num_faces*3); i++) {
        fdata.push_back( _freadt<int32_t>(is));
      }
      is.close();
      surface->vertices = vdata;
      surface->faces = fdata;
    } else {
      throw std::runtime_error("Unable to open surface file '" + filename + "'.\n");
    }
  }


  /// @brief Read a triangular mesh from a surf, obj, or ply file into the given Mesh instance.
  ///
  /// @param surface a Mesh instance representing a vertex-indexed tri-mesh. This will be filled.
  /// @param filename The path to the file from which to read the mesh. The format will be determined from the file extension as follows. File names ending with '.obj' are loaded as Wavefront OBJ files. File names ending with '.ply' are loaded as Stanford PLY files in format version 'ascii 1.0'. All other files are loaded as FreeSurfer binary surf files.
  /// @throws runtime_error if the file cannot be opened, domain_error if the surf file magic mismatches.
  ///
  /// #### Examples
  ///
  /// @code
  /// fs::mesh surface;
  /// fs::read_mesh(&surface, "subject1/surf/lh.thickness");
  /// @endcode
  void read_mesh(Mesh* surface, const std::string& filename) {
    if(fs::util::ends_with(filename, ".obj")) {
      fs::Mesh::from_obj(surface, filename);
    } else if(fs::util::ends_with(filename, ".ply")) {
      fs::Mesh::from_ply(surface, filename);
    } else if(fs::util::ends_with(filename, ".off")) {
      fs::Mesh::from_off(surface, filename);
    } else {
      read_surf(surface, filename);
    }
  }


  /// @brief Determine the endianness of the system.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @return boolean, whether the current system is big endian.
  /// @private
  bool _is_bigendian() {
    short int number = 0x1;
    char *numPtr = (char*)&number;
    //std::cout << "Platform is big endian: " << (numPtr[0] != 1) << ".\n";
    return (numPtr[0] != 1);
  }

  /// @brief Read per-vertex brain morphometry data from a FreeSurfer curv stream.
  /// @details The curv format is a simple binary format that stores one floating point value per vertex of a related brain surface.
  /// @param curv A Curv instance to be filled.
  /// @param is An open istream from which to read the curv data.
  /// @throws domain_error if the curv file magic mismatches or the curv file header claims that the file contains more than 1 value per vertex.
  void read_curv(Curv* curv, std::istream *is, const std::string& source_filename="") {
    const std::string msg_source_file_part = source_filename.empty() ? "" : "'" + source_filename + "' ";
    const int CURV_MAGIC = 16777215;
    int magic = _fread3(*is);
    if(magic != CURV_MAGIC) {
      throw std::domain_error("Curv file " + msg_source_file_part + "header magic did not match: expected " + std::to_string(CURV_MAGIC) + ", found " + std::to_string(magic) + ".\n");
    }
    curv->num_vertices = _freadt<int32_t>(*is);
    curv->num_faces =  _freadt<int32_t>(*is);
    curv->num_values_per_vertex = _freadt<int32_t>(*is);
    #ifdef LIBFS_DBG_INFO
    std::cout << LIBFS_APPTAG << "Read curv file with " << curv->num_vertices << " vertices, " << curv->num_faces << " faces and " << curv->num_values_per_vertex << " values per vertex.\n";
    #endif
    if(curv->num_values_per_vertex != 1) { // Not supported, I know no case where this is used. Please submit a PR with a demo file if you have one, and let me know where it came from.
      throw std::domain_error("Curv file " + msg_source_file_part + "must contain exactly 1 value per vertex, found " + std::to_string(curv->num_values_per_vertex) + ".\n");
    }
    std::vector<float> data;
    for(int i=0; i<curv->num_vertices; i++) {
      data.push_back( _freadt<float>(*is));
    }
    curv->data = data;
  }


  /// @brief Read Curv instance from a FreeSurfer curv format file.
  /// @details The curv format is a simple binary format that stores one floating point value per vertex of a related brain surface.
  /// @param curv A Curv instance to be filled.
  /// @param filename Path to a file from which to read the curv data.
  /// @throws runtime_error if the file cannot be opened, domain_error if the curv file magic mismatches or the curv file header claims that the file contains more than 1 value per vertex.
  ///
  /// #### Examples
  ///
  /// @code
  /// fs::Curv curv;
  /// fs::read_curv(&curv, "examples/read_curv/lh.thickness");
  /// @endcode
  void read_curv(Curv* curv, const std::string& filename) {
    std::ifstream is(filename, std::fstream::in | std::fstream::binary);
    if(is.is_open()) {
      read_curv(curv, &is, filename);
      is.close();
    } else {
      throw std::runtime_error("Could not open curv file '" + filename + "' for reading.\n");
    }
  }

  /// Read an Annot Colortable from a stream.
  /// @private
  void _read_annot_colortable(Colortable* colortable, std::istream *is, int32_t num_entries) {
    int32_t num_chars_orig_filename = _freadt<int32_t>(*is);  // The number of characters of the file this annot was built from.

    // It follows the name of the file this annot was built from. This is development metadata and irrelevant afaik. We skip it.
    uint8_t discarded;
    for(int32_t i=0; i<num_chars_orig_filename; i++) {
      discarded = _freadt<uint8_t>(*is);
    }
    (void)discarded; // Suppress warnings about unused variable.

    int32_t num_entries_duplicated = _freadt<int32_t>(*is); // Yes, once more.
    if(num_entries != num_entries_duplicated) {
      std::cerr << "Warning: the two num_entries header fields of this annotation do not match. Use with care.\n";
    }

    int32_t entry_num_chars;
    for(int32_t i=0; i<num_entries; i++) {
      colortable->id.push_back(_freadt<int32_t>(*is));
      entry_num_chars = _freadt<int32_t>(*is);
      colortable->name.push_back(_freadfixedlengthstring(*is, entry_num_chars, true));
      colortable->r.push_back(_freadt<int32_t>(*is));
      colortable->g.push_back(_freadt<int32_t>(*is));
      colortable->b.push_back(_freadt<int32_t>(*is));
      colortable->a.push_back(_freadt<int32_t>(*is));
      colortable->label.push_back(colortable->r[i] + colortable->g[i]*256 + colortable->b[i]*65536 + colortable->a[i]*16777216);
    }

  }

  /// Compute the vector index for treating a vector of length n*m as a matrix with n rows and m columns.
  /// @private
  size_t _vidx_2d(size_t row, size_t column, size_t row_length=3) {
    return (row+1)*row_length -row_length + column;
  }

  /// @brief Read a FreeSurfer annotation or brain surface parcellation from an annot stream.
  /// @details A brain parcellations contains a region table and assigns to each vertex of a surface a region.
  /// @param annot An Annot instance to be filled.
  /// @param is An open istream from which to read the annot data.
  /// @throws domain_error if the file format version is not supported or the file is missing the color table.
  void read_annot(Annot* annot, std::istream *is) {

    int32_t num_vertices = _freadt<int32_t>(*is);
    std::vector<int32_t> vertices;
    std::vector<int32_t> labels;
    for(int32_t i=0; i<(num_vertices*2); i++) { // The vertices and their labels are stored directly after one another: v1,v1_label,v2,v2_label,...
        if(i % 2 == 0) {
          vertices.push_back(_freadt<int32_t>(*is));
        } else {
          labels.push_back(_freadt<int32_t>(*is));
        }
    }
    annot->vertex_indices = vertices;
    annot->vertex_labels = labels;
    int32_t has_colortable = _freadt<int32_t>(*is);
    if(has_colortable == 1) {
      int32_t num_colortable_entries_old_format = _freadt<int32_t>(*is);
      if(num_colortable_entries_old_format > 0) {
        throw std::domain_error("Reading annotation in old format not supported. Please open an issue and supply an example file if you need this.\n");
      } else {
        int32_t colortable_format_version = -num_colortable_entries_old_format; // If the value is negative, we are in new format and its absolute value is the format version.
        if(colortable_format_version == 2) {
          int32_t num_colortable_entries = _freadt<int32_t>(*is); // This time for real.
          _read_annot_colortable(&annot->colortable, is, num_colortable_entries);
        } else {
          throw std::domain_error("Reading annotation in new format version !=2 not supported. Please open an issue and supply an example file if you need this.\n");
        }

      }

    } else {
      throw std::domain_error("Reading annotation without colortable not supported. Maybe invalid annotation file?\n");
    }
  }


  /// @brief Read a FreeSurfer annotation from a file.
  /// @param annot An Annot instance that should be filled.
  /// @param filename Path to the label file that should be read.
  /// @see There exists an overload to read from a stream instead.
  /// @throws runtime_error if the file cannot be opened, domain_error if the file format version is not supported or the file is missing the color table.
  ///
  /// #### Examples
  ///
  /// @code
  /// std::string annot_fname = "lh.aparc.annot";
  /// fs::Annot annot;
  /// fs::read_annot(&annot, annot_fname);
  /// @endcode
  void read_annot(Annot* annot, const std::string& filename) {
    std::ifstream is(filename, std::fstream::in | std::fstream::binary);
    if(is.is_open()) {
      read_annot(annot, &is);
      is.close();
    } else {
      throw std::runtime_error("Could not open annot file '" + filename + "' for reading.\n");
    }
  }


  /// @brief Read per-vertex brain morphometry data from a FreeSurfer curv format file.
  /// @details The curv format is a simple binary format that stores one floating point value per vertex of a related brain surface.
  /// @param filename Path to a file from which to read the curv data.
  /// @return a vector of float values, one per vertex.
  /// @throws runtime_error if the file cannot be opened, domain_error if the curv file magic mismatches or the curv file header claims that the file contains more than 1 value per vertex.
  ///
  /// #### Examples
  ///
  /// @code
  /// std::string curv_fname = "lh.thickness";
  /// std::vector<float> data = fs::read_curv_data(curv_fname);
  /// @endcode
  std::vector<float> read_curv_data(const std::string& filename) {
    Curv curv;
    read_curv(&curv, filename);
    return(curv.data);
  }


  /// Swap endianness of a value.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  template <typename T>
  T _swap_endian(T u) {

      static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

      union
      {
          T u;
          unsigned char u8[sizeof(T)];
      } source, dest;

      source.u = u;

      for (size_t k = 0; k < sizeof(T); k++)
          dest.u8[k] = source.u8[sizeof(T) - k - 1];

      return(dest.u);
  }

  /// Read a big endian value from a stream.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  template <typename T>
  T _freadt(std::istream& is) {
    T t;
    is.read(reinterpret_cast<char*>(&t), sizeof(t));
    if(! _is_bigendian()) {
      t = _swap_endian<T>(t);
    }
    return(t);
  }

  /// Read 3 big endian bytes as a single integer from a stream.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  int _fread3(std::istream& is) {
    uint32_t i;
    is.read(reinterpret_cast<char*>(&i), 3);
    if(! _is_bigendian()) {
      i = _swap_endian<std::uint32_t>(i);
    }
    i = ((i >> 8) & 0xffffff);
    return(i);
  }

  /// Write a value to a stream as big endian.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  template <typename T>
  void _fwritet(std::ostream& os, T t) {
    if(! _is_bigendian()) {
      t = _swap_endian<T>(t);
    }
    os.write( reinterpret_cast<const char*>( &t ), sizeof(t));
  }


  // Write big endian 24 bit integer to a stream, extracted from the first 3 bytes of an unsigned 32 bit integer.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  void _fwritei3(std::ostream& os, uint32_t i) {
    unsigned char b1 = ( i >> 16) & 255;
    unsigned char b2 = ( i >> 8) & 255;
    unsigned char b3 =  i & 255;

    if(!_is_bigendian()) {
      b1 = _swap_endian<unsigned char>(b1);
      b2 = _swap_endian<unsigned char>(b2);
      b3 = _swap_endian<unsigned char>(b3);
    }

    os.write( reinterpret_cast<const char*>( &b1 ), sizeof(b1));
    os.write( reinterpret_cast<const char*>( &b2 ), sizeof(b2));
    os.write( reinterpret_cast<const char*>( &b3 ), sizeof(b3));
  }

  /// Read a '\n'-terminated ASCII string from a stream.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @private
  std::string _freadstringnewline(std::istream &is) {
    std::string s;
    std::getline(is, s, '\n');
    return s;
  }

  /// Read a fixed length C-style string from an open binary stream. This does not care about trailing NULL bytes or anything, it just reads the given length of bytes.
  /// @throws std::out_of_range if length is not positive
  /// @private
  std::string _freadfixedlengthstring(std::istream &is, size_t length, bool strip_last_char=true) {
    std::string str;
    str.resize(length);
    is.read(&str[0], length);
    if(strip_last_char) {
      str = str.substr(0, length-1);
    }
    return str;
  }


  /// @brief Write curv data to a stream.
  /// @details A curv file contains one floating point value per vertex (or a related mesh).
  /// @param os An output stream to which to write the data. The stream must be open, and this function will not close it after writing to it.
  /// @param curv_data the data to write.
  /// @param num_faces the value for the header field `num_faces`. This is not needed afaik and typically ignored.
  void write_curv(std::ostream& os, std::vector<float> curv_data, int32_t num_faces = 100000) {
    const uint32_t CURV_MAGIC = 16777215;
    _fwritei3(os, CURV_MAGIC);
    _fwritet<int32_t>(os, int(curv_data.size()));
    _fwritet<int32_t>(os, num_faces);
    _fwritet<int32_t>(os, 1); // Number of values per vertex.
    for(size_t i=0; i<curv_data.size(); i++) {
       _fwritet<float>(os, curv_data[i]);
    }
  }


  /// @brief Write curv data to a file.
  /// @details A curv file contains one floating point value per vertex (or a related mesh).
  /// @param filename The path to the output file.
  /// @param curv_data the data to write.
  /// @param num_faces the value for the header field `num_faces`. This is not needed afaik and typically ignored.
  /// @throws std::runtime_error if the file cannot be opened.
  ///
  /// #### Examples
  ///
  /// @code
  /// std::vector<float> data = fs::read_curv_data("lh.thickness");
  /// // Do something with 'data' here, maybe?
  /// fs::write_curv("output.curv", data);
  /// @endcode
  void write_curv(const std::string& filename, std::vector<float> curv_data, const int32_t num_faces = 100000) {
    std::ofstream ofs;
    ofs.open(filename, std::ofstream::out | std::ofstream::binary);
    if(ofs.is_open()) {
      write_curv(ofs, curv_data, num_faces);
      ofs.close();
    } else {
      throw std::runtime_error("Unable to open curvature file '" + filename + "' for writing.\n");
    }
  }

  /// @brief Write MGH data to a stream.
  /// @details The MGH format is a binary, big-endian FreeSurfer file format for storing 4D data. Several data types are supported, and one has to check the header to see which one is contained in a file.
  /// @param mgh An Mgh instance that should be written.
  /// @param os An output stream to which to write the data. The stream must be open, and this function will not close it after writing to it.
  /// @throws std::logic_error if the mgh header and data are inconsistent, std::domain_error if the given MRI data type is unknown or unsupported.
  void write_mgh(const Mgh& mgh, std::ostream& os) {
     _fwritet<int32_t>(os, 1); // MGH file format version
     _fwritet<int32_t>(os, mgh.header.dim1length);
     _fwritet<int32_t>(os, mgh.header.dim2length);
     _fwritet<int32_t>(os, mgh.header.dim3length);
     _fwritet<int32_t>(os, mgh.header.dim4length);

     _fwritet<int32_t>(os, mgh.header.dtype);
     _fwritet<int32_t>(os, mgh.header.dof);

    size_t unused_header_space_size_left = 256;  // in bytes
     _fwritet<int16_t>(os, mgh.header.ras_good_flag);
    unused_header_space_size_left -= 2; // for RAS flag

    // Write RAS part of of header if flag is 1.
    if(mgh.header.ras_good_flag == 1) {
       _fwritet<float>(os, mgh.header.xsize);
       _fwritet<float>(os, mgh.header.ysize);
       _fwritet<float>(os, mgh.header.zsize);

      for(int i=0; i<9; i++) {
         _fwritet<float>(os, mgh.header.Mdc[i]);
      }
      for(int i=0; i<3; i++) {
         _fwritet<float>(os, mgh.header.Pxyz_c[i]);
      }

      unused_header_space_size_left -= 60;
    }

    for(size_t i=0; i<unused_header_space_size_left; i++) {  // Fill rest of header space.
       _fwritet<uint8_t>(os, 0);
    }

    // Write data
    size_t num_values = mgh.header.num_values();
    if(mgh.header.dtype == MRI_INT) {
      if(mgh.data.data_mri_int.size() != num_values) {
        throw std::logic_error("Detected mismatch of MRI_INT data size and MGH header dim length values.\n");
      }
      for(size_t i=0; i<num_values; i++) {
         _fwritet<int32_t>(os, mgh.data.data_mri_int[i]);
      }
    } else if(mgh.header.dtype == MRI_FLOAT) {
      if(mgh.data.data_mri_float.size() != num_values) {
        throw std::logic_error("Detected mismatch of MRI_FLOAT data size and MGH header dim length values.\n");
      }
      for(size_t i=0; i<num_values; i++) {
         _fwritet<float>(os, mgh.data.data_mri_float[i]);
      }
    } else if(mgh.header.dtype == MRI_UCHAR) {
      if(mgh.data.data_mri_uchar.size() != num_values) {
        throw std::logic_error("Detected mismatch of MRI_UCHAR data size and MGH header dim length values.\n");
      }
      for(size_t i=0; i<num_values; i++) {
         _fwritet<uint8_t>(os, mgh.data.data_mri_uchar[i]);
      }
    } else if(mgh.header.dtype == MRI_SHORT) {
      if(mgh.data.data_mri_short.size() != num_values) {
        throw std::logic_error("Detected mismatch of MRI_SHORT data size and MGH header dim length values.\n");
      }
      for(size_t i=0; i<num_values; i++) {
         _fwritet<short>(os, mgh.data.data_mri_short[i]);
      }
    } else {
      throw std::domain_error("Unsupported MRI data type " + std::to_string(mgh.header.dtype) + ", cannot write MGH data.\n");
    }

  }

  /// @brief Write MGH data to a file.
  /// @details The MGH format is a binary, big-endian FreeSurfer file format for storing 4D data. Several data types are supported, and one has to check the header to see which one is contained in a file.
  /// @param mgh An Mgh instance that should be written.
  /// @param filename Path to an output file to which to write.
  /// @see There exists an overload to write to a stream.
  /// @throws std::runtime_error if the file cannot be opened, std::logic_error if the mgh header and data are inconsistent, std::domain_error if the given MRI data type is unknown or unsupported.
  ///
  /// #### Examples
  ///
  /// @code
  /// fs::Mgh mgh;
  /// fs::read_mgh(&mgh, "somebrain.mgh");
  /// // Do something with 'mgh' here, maybe?
  /// fs::write_mgh(mgh, "output.mgh");
  /// @endcode
  void write_mgh(const Mgh& mgh, const std::string& filename) {
    std::ofstream ofs;
    ofs.open(filename, std::ofstream::out | std::ofstream::binary);
    if(ofs.is_open()) {
      write_mgh(mgh, ofs);
      ofs.close();
    } else {
      throw std::runtime_error("Unable to open MGH file '" + filename + "' for writing.\n");
    }
  }

  /// Models a FreeSurfer label.
  /// Can be a surface or volume label.
  /// A label contains entries for a subset of the vertices of a mesh (or the voxels of a volume).
  /// For a surface label, the 'vertex' field contains a vertex index, the coord_* fields may contain the vertex coordinates, and the value field may contain some per-vertex descriptor or analysis result, like a p-value.
  /// For a volume label, the 'vertex' field contains a running number, the coord_* fields contain (as floats) the voxel R,A,S indices, and the value field may contain some per-voxel descriptor or analysis result, like a p-value.
  /// Note: The variable names herre assume this is a surface label, do not let that confuse you when working with volume labels.
  struct Label {

    /// @brief Default constructor for a label.
    Label() {}

    /// Construct a Label from the given vertices / voxel numbers and values.
    Label(std::vector<int> vertices, std::vector<float> values) {
      assert(vertices.size() == values.size());
      vertex = vertices;
      value = values;
      coord_x = std::vector<float>(vertices.size(), 0.0f);
      coord_y = std::vector<float>(vertices.size(), 0.0f);
      coord_z = std::vector<float>(vertices.size(), 0.0f);
    }

    /// Construct a Label from the given vertices / voxel numbers.
    Label(std::vector<int> vertices) {
      vertex = vertices;
      value = std::vector<float>(vertices.size(), 0.0f);
      coord_x = std::vector<float>(vertices.size(), 0.0f);
      coord_y = std::vector<float>(vertices.size(), 0.0f);
      coord_z = std::vector<float>(vertices.size(), 0.0f);
    }



    std::vector<int> vertex;  ///< vertex indices for the data in this label if it is a surface label. These are indices into the vertices of a surface mesh to which this label belongs.
    std::vector<float> coord_x;  ///< x coordinates of the vertices in case of a surface label, or voxels coordinates for a volume label.
    std::vector<float> coord_y;  ///< y coordinates of the vertices in case of a surface label, or voxels coordinates for a volume label.
    std::vector<float> coord_z;  ///< z coordinates of the vertices in case of a surface label, or voxels coordinates for a volume label.
    std::vector<float> value;  ///< the value of the label, can represent continuous data like a p-value, or sometimes simply 1.0 or 0.0 (interpreted as int/bool) to mark vertices inside/outside of a certain area.

    /// Compute for each vertex of the surface whether it is inside the label.
    std::vector<bool> vert_in_label(size_t surface_num_verts) const {
      if(surface_num_verts < this->vertex.size()) { // nonsense, so we warn (but don't throw, maybe the user really wants this).
        std::cerr << "Invalid number of vertices for surface, must be at least " << this->vertex.size() << "\n";
      }
      std::vector<bool> is_in = std::vector<bool>(surface_num_verts, false);

      for(size_t i=0; i < this->vertex.size(); i++) {
        is_in[this->vertex[i]] = true;
      }
      return(is_in);
    }

    /// Return the number of entries (vertices/voxels) in this label.
    size_t num_entries() const {
      size_t num_ent = this->vertex.size();
      if(this->coord_x.size() != num_ent || this->coord_y.size() != num_ent || this->coord_z.size() != num_ent || this->value.size() != num_ent) {
        std::cerr << "Inconsistent label: sizes of property vectors do not match.\n";
      }
      return(num_ent);
    }
  };

  /// @brief Write a mesh to a stream in FreeSurfer surf format.
  /// @details A surf file contains a vertex index representation of a mesh, i.e., the vertices and faces vectors.
  /// @param vertices vector of float, length 3n for n vertices. The 3D coordinates of the vertices, typically from `<Mesh_instance>.vertices`.
  /// @param faces vector of int, length 3n for n faces. The 3 vertex indices for each face, typically from `<Mesh_instance>.faces`.
  /// @param os An output stream to which to write the data. The stream must be open, and this function will not close it after writing to it.
  /// @throws std::runtime_error if the file cannot be opened.
  void write_surf(std::vector<float> vertices, std::vector<int32_t> faces, std::ostream& os) {
    const uint32_t SURF_TRIS_MAGIC = 16777214;
    _fwritei3(os, SURF_TRIS_MAGIC);
    std::string created_and_comment_lines = "Created by fslib\n\n";
    os << created_and_comment_lines;
    _fwritet<int32_t>(os, int(vertices.size() / 3));  // number of vertices
    _fwritet<int32_t>(os, int(faces.size() / 3));  // number of faces
    for(size_t i=0; i < vertices.size(); i++) {
       _fwritet<float>(os, vertices[i]);
    }
    for(size_t i=0; i < faces.size(); i++) {
       _fwritet<int32_t>(os, faces[i]);
    }
  }

  /// @brief Write a mesh to a binary file in FreeSurfer surf format.
  /// @details A surf file contains a vertex index representation of a mesh, i.e., the vertices and faces vectors.
  /// @param vertices vector of float, length 3n for n vertices. The 3D coordinates of the vertices, typically from `<Mesh_instance>.vertices`.
  /// @param faces vector of int, length 3n for n faces. The 3 vertex indices for each face, typically from `<Mesh_instance>.faces`.
  /// @param filename The path to the output file.
  /// @throws std::runtime_error if the file cannot be opened.
  ///
  /// #### Examples
  ///
  /// @code
  /// fs::Mesh surface = fs::Mesh::construct_cube();
  /// fs::write_surf(surface.vertices, surface.faces, "lh.cube");
  /// @endcode
  void write_surf(std::vector<float> vertices, std::vector<int32_t> faces, const std::string& filename) {
    std::ofstream ofs;
    ofs.open(filename, std::ofstream::out | std::ofstream::binary);
    if(ofs.is_open()) {
      write_surf(vertices, faces, ofs);
      ofs.close();
    } else {
      throw std::runtime_error("Unable to open surf file '" + filename + "' for writing.\n");
    }
  }


  /// @brief Write a mesh to a binary file in FreeSurfer surf format.
  /// @details A surf file contains a vertex index representation of a mesh, i.e., the vertices and faces vectors.
  /// @param mesh The `Mesh` instance to write.
  /// @param filename The path to the output file.
  /// @throws std::runtime_error if the file cannot be opened.
  ///
  /// #### Examples
  ///
  /// @code
  /// fs::Mesh surface = fs::Mesh::construct_cube();
  /// fs::write_surf(surface, "lh.cube");
  /// @endcode
  void write_surf(const Mesh& mesh, const std::string& filename ) {
    std::ofstream ofs;
    ofs.open(filename, std::ofstream::out | std::ofstream::binary);
    if(ofs.is_open()) {
      write_surf(mesh.vertices, mesh.faces, ofs);
      ofs.close();
    } else {
      throw std::runtime_error("Unable to open surf file '" + filename + "' for writing.\n");
    }
  }

  /// @brief Read a FreeSurfer ASCII label from a stream.
  /// @details A label is a list of vertices (for a surface label, given by index) or voxels (for a volume label, given by the xyz coordinates) and one floating point value per vertex/voxel. Sometimes a label is only used to define a set of vertices/voxels (like a certain brain region), and the values are irrelevant (and typically left at 0.0).
  /// @param label A Label instance that should be filled.
  /// @param is An open std::istream or derived class stream from which to read the data, e.g., std::ifstream or std::istringstream.
  /// @see There exists an overload to read from a file instead.
  /// @throws std::domain_error if the label data format is incorrect
  void read_label(Label* label, std::istream* is) {
    std::string line;
    int line_idx = -1;
    size_t num_entries_header = 0;  // number of vertices/voxels according to header
    size_t num_entries = 0;  // number of vertices/voxels for which the file contains label entries.
    while (std::getline(*is, line)) {
      line_idx += 1;
      std::istringstream iss(line);
      if(line_idx == 0) {
        continue; // skip comment.
      } else {
        if(line_idx == 1) {
          if (!(iss >> num_entries_header)) {
            throw std::domain_error("Could not parse entry count from label file, invalid format.\n");
          }
        } else {
          int vertex; float x, y, z, value;
          if (!(iss >> vertex >> x >> y >> z >> value)) {
            throw std::domain_error("Could not parse line " + std::to_string(line_idx+1) + " of label file, invalid format.\n");
          }
          label->vertex.push_back(vertex);
          label->coord_x.push_back(x);
          label->coord_y.push_back(y);
          label->coord_z.push_back(z);
          label->value.push_back(value);
          num_entries++;
        }
      }
    }
    if(num_entries != num_entries_header) {
      throw std::domain_error("Expected " + std::to_string(num_entries_header) + " entries from label file header, but found " + std::to_string(num_entries) + " in file, invalid label file.\n");
    }
    if(label->vertex.size() != num_entries || label->coord_x.size() != num_entries || label->coord_y.size() != num_entries || label->coord_z.size() != num_entries || label->value.size() != num_entries) {
      throw std::domain_error("Expected " + std::to_string(num_entries) + " entries in all Label vectors, but some did not match.\n");
    }
  }


  /// @brief Read a FreeSurfer ASCII label from a file.
  /// @details A label is a list of vertices (for a surface label, given by index) or voxels (for a volume label, given by the xyz coordinates) and one floating point value per vertex/voxel. Sometimes a label is only used to define a set of vertices/voxels (like a certain brain region), and the values are irrelevant (and typically left at 0.0).
  /// @param label A Label instance that should be filled.
  /// @param filename Path to the label file that should be read.
  /// @see There exists an overload to read from a stream instead.
  /// @throws std::domain_error if the label data format is incorrect, std::runtime_error if the file cannot be opened.
  ///
  /// #### Examples
  ///
  /// @code
  /// fs::Label label;
  /// fs::read_label(&label, "subject1/label/lh.cortex.label");
  /// @endcode
  void read_label(Label* label, const std::string& filename) {
    std::ifstream infile(filename, std::fstream::in);
    if(infile.is_open()) {
      read_label(label, &infile);
      infile.close();
    } else {
      throw std::runtime_error("Could not open label file '" + filename + "' for reading.\n");
    }
  }


  /// @brief Write label data to a stream.
  /// @param label The label to write.
  /// @param os An open output stream.
  /// @see There exists an onverload of this function to write a label to a file.
  void write_label(const Label& label, std::ostream& os) {
    const size_t num_entries = label.num_entries();
    os << "#!ascii label from subject anonymous\n" << num_entries << "\n";
    for(size_t i=0; i<num_entries; i++) {
      os << label.vertex[i] << " " << label.coord_x[i] << " " << label.coord_y[i] << " " << label.coord_z[i] << " " << label.value[i] << "\n";
    }
  }


  /// @brief Write label data to a file.
  /// @param label an fs::Label instance
  /// @param filename Path to the label file that should be written.
  /// @see There exists an overload to write to a stream.
  /// @throws std::runtime_error if the file cannot be opened.
  ///
  /// #### Examples
  ///
  /// @code
  /// fs::Label label;
  /// fs::read_label(&label, "subject1/label/lh.cortex.label");
  /// fs::write_label(label, "out.label");
  /// @endcode
  void write_label(const Label& label, const std::string& filename) {
    std::ofstream ofs;
    ofs.open(filename, std::ofstream::out);
    if(ofs.is_open()) {
      write_label(label, ofs);
      ofs.close();
    } else {
      throw std::runtime_error("Unable to open label file '" + filename + "' for writing.\n");
    }
  }

  /// @brief Write a mesh to a file in different formats.
  /// @details The output format will be auto-determined from the file extension.
  /// @param mesh The fs::Mesh instance to write.
  /// @param filename The path to the output file.
  /// @throws std::runtime_error if the file cannot be opened.
  ///
  /// #### Examples
  ///
  /// @code
  /// fs::Mesh surface = fs::Mesh::construct_cube();
  /// fs::write_mesh(surface, "cube.ply");
  /// fs::write_mesh(surface, "cube.off");
  /// fs::write_mesh(surface, "cube.obj");
  /// fs::write_mesh(surface, "cube");  // writes FS surf format.
  /// @endcode
  void write_mesh(const Mesh& mesh, const std::string& filename ) {
    if (fs::util::ends_with(filename, {".ply", ".PLY"})) {
      mesh.to_ply_file(filename);
    } else if (fs::util::ends_with(filename, {".obj", ".OBJ"})) {
      mesh.to_obj_file(filename);
    } else if (fs::util::ends_with(filename, {".off", ".OFF"})) {
      mesh.to_off_file(filename);
    } else {
      fs::write_surf(mesh, filename);
    }
  }

} // End namespace fs


