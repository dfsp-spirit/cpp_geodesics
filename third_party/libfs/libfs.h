#pragma once

#include <iostream>
#include <climits>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <cassert>
#include <sstream>
#include <stdexcept>

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
 * The project page for fslib can be found at https://github.com/dfsp-spirit/libfs
 * 
 */ 



namespace fs {

  namespace util {
    /// Check whether a string ends with the given suffix.
    inline bool ends_with(std::string const & value, std::string const & suffix) {
        if (suffix.size() > value.size()) return false;
        return std::equal(suffix.rbegin(), suffix.rend(), value.rbegin());
    }

    /// Check whether a string starts with the given prefix.
    inline bool starts_with(std::string const & value, std::string const & prefix) {
        if (prefix.length() > value.length()) return false;
        return value.rfind(prefix, 0) == 0;
    }


    /// Construct a UNIX file system path from the given path_components.
    /// @details Any trailing or leading slash (path_sep) will be stripped from the individual components and replaced with a single one between two components. If the first path component started with a slash, that slash will be kept (absolute paths are left intact).
    /// @param path_components init list of strings, the path components.
    /// @throws std::invalid_argument on empty path_components
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

    /// Write the given text representation (any string) to a file.
    /// @throws st::runtime_error if the file cannot be opened.
    void str_to_file(const std::string& filename, const std::string rep) {
      std::ofstream ofs;
      ofs.open(filename, std::ofstream::out);
      if(ofs.is_open()) {
          ofs << rep;
          ofs.close();
      } else {
          throw std::runtime_error("Unable to open file '" + filename + "' for writing.\n");
      }
    }
  }

  
  // MRI data types, used by the MGH functions.

  /// MRI data type representing an 8 bit unsigned integer.
  const int MRI_UCHAR = 0;

  /// MRI data type representing a 32 bit signed integer.
  const int MRI_INT = 1;

  /// MRI data type representing a 32 bit float.
  const int MRI_FLOAT = 3;

  /// MRI data type representing a 16 bit signed integer.
  const int MRI_SHORT = 4;

  // Declarations, should go to a header file.
  int _fread3(std::istream&);
  template <typename T> T _freadt(std::istream&);
  std::string _freadstringnewline(std::istream&);
  std::string _freadfixedlengthstring(std::istream&, int32_t, bool);
  bool _ends_with(std::string const &fullString, std::string const &ending);
  size_t _vidx_2d(size_t, size_t, size_t);
  struct MghHeader;
  
  /// Models a triangular mesh, used for brain surface meshes.
  /// 
  /// Represents a vertex-indexed mesh. The n vertices are stored as 3D point coordinates (x,y,z) in a vector
  /// of length 3n, in which 3 consecutive values represent the x, y and z coordinate of the same vertex.
  /// The m faces are stored as a vector of 3m integers, where 3 consecutive values represent the 3 vertices (by index)
  /// making up the respective face. Vertex indices are 0-based.
  struct Mesh {

    /// Construct a Mesh from the given vertices and faces.
    Mesh(std::vector<float> cvertices, std::vector<int32_t> cfaces) { 
      vertices = cvertices; faces = cfaces; 
    }

    /// Construct an empty Mesh.
    Mesh() {}

    std::vector<float> vertices;
    std::vector<int32_t> faces;    


    /// Return string representing the mesh in Wavefront Object (.obj) format.
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


    /// Export this mesh to a file in Wavefront OBJ format.
    /// @throws st::runtime_error if the target file cannot be opened.
    void to_obj_file(const std::string& filename) const {
      fs::util::str_to_file(filename, this->to_obj());
    }
    

    /// Read a brainmesh from a Wavefront object format stream.
    /// @details This only reads the geometry, optional format extensions like materials are ignored (but files including them should parse fine).
    /// @throws std::domain_error if the file format is invalid.
    static void from_obj(Mesh* mesh, std::ifstream* is) {
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
      if(num_lines_ignored > 0) {
        std::cerr << "Ignored " << num_lines_ignored << " lines in Wavefront OBJ format mesh file.\n";
      }
      mesh->vertices = vertices;
      mesh->faces = faces;
    }


    /// Read a brainmesh from a Wavefront object format mesh file.
    /// @details This only reads the geometry, optional format extensions like materials are ignored (but files including them should parse fine).
    /// @throws std::runtime_error if the file cannot be read.
    /// @throws std::domain_error if the file format is invalid.
    static void from_obj(Mesh* mesh, const std::string& filename) {
      std::ifstream input(filename);
      if(input.is_open()) {
        Mesh::from_obj(mesh, &input);
        input.close();
      } else {
        throw std::runtime_error("Could not open Wavefront object format mesh file '" + filename + "' for reading.\n");
      }
    }


    /// Read a brainmesh from an Object File format (OFF) stream.
    /// @throws std::domain_error if the file format is invalid.
    static void from_off(Mesh* mesh, std::ifstream* is) {
      std::string line;
      int line_idx = -1;
      int noncomment_line_idx = -1;

      std::vector<float> vertices;
      std::vector<int> faces;
      size_t num_vertices, num_faces, num_edges;
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
              throw std::domain_error("OFF magic string invalid, file not in OFF format.\n");
            }
            //has_color = off_header_magic == "COFF";
          } else if (noncomment_line_idx == 1) {
            if (!(iss >> num_vertices >> num_faces >> num_edges)) {
              throw std::domain_error("Could not parse element count header line " + std::to_string(line_idx+1) + " of OFF data, invalid format.\n");
            }
          } else {
            
            if(num_verts_parsed < num_vertices) {
              if (!(iss >> x >> y >> z)) {
                throw std::domain_error("Could not parse vertex coordinate line " + std::to_string(line_idx+1) + " of OFF data, invalid format.\n");
              }
              vertices.push_back(x);
              vertices.push_back(y);
              vertices.push_back(z);
              num_verts_parsed++;
            } else {
              if(num_faces_parsed < num_faces) {
                if (!(iss >> num_verts_this_face >> v0 >> v1 >> v2)) {
                  throw std::domain_error("Could not parse face line " + std::to_string(line_idx+1) + " of OFF data, invalid format.\n");
                }
                if(num_verts_this_face != 3) {
                  throw std::domain_error("At OFF line " + std::to_string(line_idx+1) + ": only triangular meshes supported.\n");
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
        throw std::domain_error("Vertex count mismatch between OFF header (" + std::to_string(num_vertices) + ") and data (" + std::to_string(num_verts_parsed) + ").\n");
      }
      if(num_faces_parsed < num_faces) {
        throw std::domain_error("Face count mismatch between OFF header  (" + std::to_string(num_faces) + ") and data (" + std::to_string(num_faces_parsed) + ").\n");
      }
      mesh->vertices = vertices;
      mesh->faces = faces;
    }


    /// Read a brainmesh from an OFF format mesh file.
    /// @details The OFF is the Object File Format (file extension .off) is a simple text-based mesh file format. Not to be confused with the Wavefront Object format (.obj).
    /// @throws std::runtime_error if the file cannot be read.
    /// @throws std::domain_error if the file format is invalid.
    static void from_off(Mesh* mesh, const std::string& filename) {
      std::ifstream input(filename);
      if(input.is_open()) {
        Mesh::from_off(mesh, &input);
        input.close();
      } else {
        throw std::runtime_error("Could not open Object file format (OFF) mesh file '" + filename + "' for reading.\n");
      }
    }


    /// Read a brainmesh from a Stanford PLY format stream.
    /// @throws std::domain_error if the file format is invalid.
    static void from_ply(Mesh* mesh, std::ifstream* is) {
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
                throw std::domain_error("Could not parse vertex line of PLY data, invalid format.\n");
              }
              vertices.push_back(x);
              vertices.push_back(y);
              vertices.push_back(z);
            } else {
              if(faces.size() < (size_t)num_faces * 3) {
                int verts_per_face, v0, v1, v2;
                if (!(iss >> verts_per_face >> v0 >> v1 >> v2)) {
                  throw std::domain_error("Could not parse face line of PLY data, invalid format.\n");
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

    /// Read a brainmesh from a Stanford PLY format mesh file.
    /// @throws std::runtime_error if the file cannot be read.
    /// @throws std::domain_error if the file format is invalid.
    static void from_ply(Mesh* mesh, const std::string& filename) {
      std::ifstream input(filename);
      if(input.is_open()) {
        Mesh::from_ply(mesh, &input);
        input.close();
      } else {
        throw std::runtime_error("Could not open Stanford PLY format mesh file '" + filename + "' for reading.\n");
      }
    }


    /// Return the number of vertices in this mesh.
    size_t num_vertices() const {
      return(this->vertices.size() / 3);
    }

    /// Return the number of faces in this mesh.
    size_t num_faces() const {
      return(this->faces.size() / 3);
    }

    /// @brief Retrieve a vertex index of a face, treating the faces vector as an nx3 matrix.
    /// @param i the row index, valid values are 0..num_faces.
    /// @param j the column index, valid values are 0..2 (for the 3 vertices of a face).
    /// @throws std::range_error on invalid index
    const int32_t& fm_at(const size_t i, const size_t j) const {
      size_t idx = _vidx_2d(i, j, 3);
      if(idx > this->faces.size()-1) {        
        throw std::range_error("Indices (" + std::to_string(i) + "," + std::to_string(j) + ") into Mesh.faces out of bounds. Hit " + std::to_string(idx) + " with max valid index " + std::to_string(this->faces.size()-1) + ".\n");
      }
      return(this->faces[idx]);
    }


    /// Get all vertex indices of the face, given by its index.
    /// @throws std::range_error on invalid index
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

    /// Get all coordinates of the vertex, given by its index.
    /// @throws std::range_error on invalid index
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

    /// @brief Retrieve a coordinate of a vertex, treating the vertices vector as an nx3 matrix.
    /// @param i the row index, valid values are 0..num_vertices.
    /// @param j the column index, valid values are 0..2 (for the x,y,z coordinates).
    /// @throws std::range_error on invalid index
    const float& vm_at(const size_t i, const size_t j) const {
      size_t idx = _vidx_2d(i, j, 3);
      if(idx > this->vertices.size()-1) {
        throw std::range_error("Indices (" + std::to_string(i) + "," + std::to_string(j) + ") into Mesh.vertices out of bounds. Hit " + std::to_string(idx) + " with max valid index " + std::to_string(this->vertices.size()-1) + ".\n");
      }
      return(this->vertices[idx]);
    }
    
    /// Return string representing the mesh in PLY format. Overload that works without passing a color vector.
    std::string to_ply() const {
      std::vector<uint8_t> empty_col;
      return(this->to_ply(empty_col));
    }

    /// Return string representing the mesh in PLY format.
    /// @param col u_char vector of RGB color values, 3 per vertex. They must appear by vertex, i.e. in order v0_red, v0_green, v0_blue, v1_red, v1_green, v1_blue. Leave empty if you do not want colors.
    /// @throws std::invalid_argument if the number of vertex colors does not match the number of vertices. 
    std::string to_ply(const std::vector<uint8_t> col) const {
      bool use_vertex_colors = col.size() != 0;
      std::stringstream plys;
      plys << "ply\nformat ascii 1.0\n";
      plys << "element vertex " << this->num_vertices() << "\n";
      plys << "property float x\nproperty float y\nproperty float z\n";
      if(use_vertex_colors) {
        if(col.size() != this->vertices.size()) {
          throw std::invalid_argument("Number of vertex coordinates and vertex colors must match when writing PLY file.");
        }
        plys << "property uchar red\nproperty uchar green\nproperty uchar blue\n";
      }
      plys << "element face " << this->num_faces() << "\n";
      plys << "property list uchar int vertex_index\n";
      plys << "end_header\n";
      
      for(size_t vidx=0; vidx<this->vertices.size();vidx+=3) {  // vertex coords
        plys << vertices[vidx] << " " << vertices[vidx+1] << " " << vertices[vidx+2];
        if(use_vertex_colors) {
          plys << " " << (int)col[vidx] << " " << (int)col[vidx+1] << " " << (int)col[vidx+2];
        }
        plys << "\n";
      }

      const int num_vertices_per_face = 3;
      for(size_t fidx=0; fidx<this->faces.size();fidx+=3) { // faces: vertex indices, 0-based
        plys << num_vertices_per_face << " " << faces[fidx] << " " << faces[fidx+1] << " " << faces[fidx+2] << "\n";
      }        
      return(plys.str());
    }

    /// Export this mesh to a file in Stanford PLY format.
    /// @throws st::runtime_error if the target file cannot be opened.
    void to_ply_file(const std::string& filename) const {
      fs::util::str_to_file(filename, this->to_ply());
    }

    /// Export this mesh to a file in Stanford PLY format with vertex colors.
    /// @throws st::runtime_error if the target file cannot be opened, std::invalid_argument if the number of vertex colors does not match the number of vertices.
    void to_ply_file(const std::string& filename, const std::vector<uint8_t> col) const {
      fs::util::str_to_file(filename, this->to_ply(col));
    }

    /// Return string representing the mesh in OFF format. Overload that works without passing a color vector.
    std::string to_off() const {
      std::vector<uint8_t> empty_col;
      return(this->to_off(empty_col));
    }

    /// Return string representing the mesh in PLY format.
    /// @param col u_char vector of RGB color values, 3 per vertex. They must appear by vertex, i.e. in order v0_red, v0_green, v0_blue, v1_red, v1_green, v1_blue. Leave empty if you do not want colors.
    /// @throws std::invalid_argument if the number of vertex colors does not match the number of vertices. 
    std::string to_off(const std::vector<uint8_t> col) const {
      bool use_vertex_colors = col.size() != 0;
      std::stringstream offs;
      if(use_vertex_colors) {
        if(col.size() != this->vertices.size()) {
          throw std::invalid_argument("Number of vertex coordinates and vertex colors must match when writing OFF file.");
        }
        offs << "COFF\n";
      } else {
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

    /// Export this mesh to a file in OFF format.
    /// @throws st::runtime_error if the target file cannot be opened.
    void to_off_file(const std::string& filename) const {
      fs::util::str_to_file(filename, this->to_off());
    }

    /// Export this mesh to a file in OFF format with vertex colors (COFF).
    /// @throws st::runtime_error if the target file cannot be opened, std::invalid_argument if the number of vertex colors does not match the number of vertices.
    void to_off_file(const std::string& filename, const std::vector<uint8_t> col) const {
      fs::util::str_to_file(filename, this->to_off(col));
    }
  };


  /// Models a FreeSurfer curv file that contains per-vertex float data.
  struct Curv {

    /// Construct a Curv instance from the given per-vertex data.
    Curv(std::vector<float> curv_data) :
      num_faces(100000), num_vertices(0), num_values_per_vertex(1) { data = curv_data; num_vertices = data.size(); }
    
    /// Construct an empty Curv instance.
    Curv() :
      num_faces(100000), num_vertices(0), num_values_per_vertex(1) {}

    int32_t num_faces;
    std::vector<float> data;
    int32_t num_vertices;
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

    /// Get the number of enties (regions) in this Colortable.
    size_t num_entries() const {
      size_t num_ids = this->id.size();
      if(this->name.size() != num_ids || this->r.size() != num_ids || this->g.size() != num_ids || this->b.size() != num_ids || this->a.size() != num_ids || this->label.size() != num_ids) {
        std::cerr << "Inconsistent Colortable, vector sizes do not match.\n";
      }
      return num_ids;
    }

    /// Get the index of a region in the Colortable by region name. Returns a negative value if the region is not found.
    int32_t get_region_idx(const std::string& query_name) const {
      for(size_t i = 0; i<this->num_entries(); i++) {
        if(this->name[i] == query_name) {
          return (int32_t)i;
        }
      }
      return(-1);
    }

    /// Get the index of a region in the Colortable by label. Returns a negative value if the region is not found.
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
    
    /// Get all vertices of a region given by name in the brain surface parcellation. Returns an integer vector, the vertex indices.
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

    /// Get all vertices of a region given by label in the brain surface parcellation. Returns an integer vector, the vertex indices.
    std::vector<int32_t> region_vertices(int32_t region_label) const {
      std::vector<int32_t> reg_verts;
      for(size_t i=0; i<this->vertex_labels.size(); i++) {
        if(this->vertex_labels[i] == region_label) {
          reg_verts.push_back(i);
        }
      }
      return(reg_verts);
    }

    /// Get the vertex colors as an array of uchar values, 3 consecutive values are the red, green and blue channel values for a single vertex.
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

    /// Get the number of vertices of this parcellation (or the associated surface).
    /// @throws std::runtime_error on invalid annot
    size_t num_vertices() const {
      size_t nv = this->vertex_indices.size();
      if(this->vertex_labels.size() != nv) {
        throw std::runtime_error("Inconsistent annot, number of vertex indices and labels does not match.\n");
      }
      return nv;
    }

    /// Compute the region indices in the Colortable for all vertices in this brain surface parcellation. With the region indices, it becomes very easy to obtain all region names, labels, and color channel values from the Colortable.
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

    /// Compute the region names in the Colortable for all vertices in this brain surface parcellation.
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
    int32_t dim1length;
    int32_t dim2length;
    int32_t dim3length;
    int32_t dim4length;

    int32_t dtype;
    int32_t dof;
    int16_t ras_good_flag;

    /// Compute the number of values based on the dim*length header fields.
    size_t num_values() const {
      return((size_t) dim1length * dim2length * dim3length * dim4length);
    }

    float xsize;
    float ysize;
    float zsize;
    std::vector<float> Mdc;
    std::vector<float> Pxyz_c;
  };

  /// Models the data of an MGH file. Currently these are 1D vectors, but one can compute the 4D array using the dimXlength fields of the respective MghHeader.
  struct MghData {
    MghData() {}
    MghData(std::vector<int32_t> curv_data) { data_mri_int = curv_data; }
    explicit MghData(std::vector<uint8_t> curv_data) { data_mri_uchar = curv_data; }
    explicit MghData(std::vector<short> curv_data) { data_mri_short = curv_data; }
    MghData(std::vector<float> curv_data) { data_mri_float = curv_data; }
    std::vector<int32_t> data_mri_int;
    std::vector<uint8_t> data_mri_uchar;
    std::vector<float> data_mri_float;
    std::vector<short> data_mri_short;
  };

  /// Models a whole MGH file.
  struct Mgh {
    MghHeader header;
    MghData data;    
  };

  /// A simple 4D array datastructure, useful for representing volume data.
  template<class T>
  struct Array4D {
    Array4D(unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4) :
      d1(d1), d2(d2), d3(d3), d4(d4), data(d1*d2*d3*d4) {}

    Array4D(MghHeader *mgh_header) :
      d1(mgh_header->dim1length), d2(mgh_header->dim2length), d3(mgh_header->dim3length), d4(mgh_header->dim4length), data(d1*d2*d3*d4) {}

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
  
    unsigned int d1;
    unsigned int d2;
    unsigned int d3;
    unsigned int d4;
    std::vector<T> data;
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


  /// Read a FreeSurfer volume file in MGH format into the given Mgh struct.
  /// 
  /// @param mgh An Mgh instance that should be filled with the data from the filename.
  /// @param filename Path to the input MGH file.
  /// @see There exists an overloaded version that reads from a stream.
  /// @throws runtime_error if the file uses an unsupported MRI data type.
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
      if(_ends_with(filename, ".mgz")) {
        std::cout << "Note: your MGH filename ends with '.mgz'. Keep in mind that MGZ format is not supported directly. You can ignore this message if you wrapped a gz stream.\n";  
      }
      throw std::runtime_error("Not reading MGH data from file '" + filename + "', data type " + std::to_string(mgh->header.dtype) + " not supported yet.\n");
    }
  }

  /// Read a vector of subject identifiers from a FreeSurfer subjects file.
  /// @param filename a text file that contains one subject identifier per line.
  /// @throws runtime_error if the file cannot be read
  std::vector<std::string> read_subjectsfile(const std::string& filename) {
    std::vector<std::string> subjects;
    std::ifstream input(filename);
    std::string line;

    if(! input.is_open()) {      
      throw std::runtime_error("Could not open subjects file '" + filename + "'.\n");
    }

    while( std::getline( input, line ) ) {
        subjects.push_back(line);
    }
    return(subjects);
  }

  /// Read MGH data from a stream.
  /// 
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

  /// Read an MGH header from a stream.
  /// 
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

  /// Read MRI_INT data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<int32_t> _read_mgh_data_int(MghHeader* mgh_header, const std::string& filename) {
    if(mgh_header->dtype != MRI_INT) {
      std::cerr << "Expected MRI data type " << MRI_INT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<int32_t>(mgh_header, filename));
  }

  /// Read MRI_INT data from a stream.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<int32_t> _read_mgh_data_int(MghHeader* mgh_header, std::istream* is) {
    if(mgh_header->dtype != MRI_INT) {
      std::cerr << "Expected MRI data type " << MRI_INT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<int32_t>(mgh_header, is));
  }

  /// Read MRI_SHORT data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<short> _read_mgh_data_short(MghHeader* mgh_header, const std::string& filename) {
    if(mgh_header->dtype != MRI_SHORT) {
      std::cerr << "Expected MRI data type " << MRI_SHORT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<short>(mgh_header, filename));
  }

  /// Read MRI_SHORT data from a stream.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<short> _read_mgh_data_short(MghHeader* mgh_header, std::istream* is) {
    if(mgh_header->dtype != MRI_SHORT) {
      std::cerr << "Expected MRI data type " << MRI_SHORT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<short>(mgh_header, is));
  }


  /// Read the header of a FreeSurfer volume file in MGH format into the given MghHeader struct.
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


  /// Read arbitrary MGH data from a file.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// @throws runtime_error if the file cannot be opened
  template <typename T>
  std::vector<T> _read_mgh_data(MghHeader* mgh_header, const std::string& filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ios_base::in | std::ios::binary);
    if(ifs.is_open()) {
      ifs.seekg(284, ifs.beg); // skip to end of header and beginning of data

      int num_values = mgh_header->num_values();
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


  /// Read arbitrary MGH data from a stream. The stream must be open and at the beginning of the MGH data.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  template <typename T>
  std::vector<T> _read_mgh_data(MghHeader* mgh_header, std::istream* is) {    
    int num_values = mgh_header->num_values();
    std::vector<T> data;
    for(int i=0; i<num_values; i++) {
      data.push_back( _freadt<T>(*is));
    }
    return(data);
  }


  /// Read MRI_FLOAT data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<float> _read_mgh_data_float(MghHeader* mgh_header, const std::string& filename) {
    if(mgh_header->dtype != MRI_FLOAT) {
      std::cerr << "Expected MRI data type " << MRI_FLOAT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<float>(mgh_header, filename));
  }

  /// Read MRI_FLOAT data from an MGH stream
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<float> _read_mgh_data_float(MghHeader* mgh_header, std::istream* is) {
    if(mgh_header->dtype != MRI_FLOAT) {
      std::cerr << "Expected MRI data type " << MRI_FLOAT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<float>(mgh_header, is));
  }

  /// Read MRI_UCHAR data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<uint8_t> _read_mgh_data_uchar(MghHeader* mgh_header, const std::string& filename) {
    if(mgh_header->dtype != MRI_UCHAR) {
      std::cerr << "Expected MRI data type " << MRI_UCHAR << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<uint8_t>(mgh_header, filename));
  }

  /// Read MRI_UCHAR data from an MGH stream
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<uint8_t> _read_mgh_data_uchar(MghHeader* mgh_header, std::istream* is) {
    if(mgh_header->dtype != MRI_UCHAR) {
      std::cerr << "Expected MRI data type " << MRI_UCHAR << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<uint8_t>(mgh_header, is));
  }

  /// Read a brain mesh from a file in binary FreeSurfer 'surf' format into the given Mesh instance.
  ///
  /// @param surface a Mesh instance representing a vertex-indexed tri-mesh. This will be filled.
  /// @param filename The path to the file from which to read the mesh. Must be in binary FreeSurfer surf format. An example file is `surf/lh.white`.
  /// @throws runtime_error if the file cannot be opened, domain_error if the surf file magic mismatches.
  /// @see read_mesh, a generalized version that supports other file formats as well.
  void read_surf(Mesh* surface, const std::string& filename) {
    const int SURF_TRIS_MAGIC = 16777214;
    std::ifstream is;
    is.open(filename, std::ios_base::in | std::ios::binary);
    if(is.is_open()) {
      int magic = _fread3(is);
      if(magic != SURF_TRIS_MAGIC) {
        throw std::domain_error("Magic did not match: expected " + std::to_string(SURF_TRIS_MAGIC) + ", found " + std::to_string(magic) + ".\n");
      }
      std::string created_line = _freadstringnewline(is);
      std::string comment_line = _freadstringnewline(is);
      int num_verts =  _freadt<int32_t>(is);
      int num_faces =  _freadt<int32_t>(is);      
      //std::cout << "Read surface file with " << num_verts << " vertices, " << num_faces << " faces.\n";
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


  /// Read a triangular mesh from a surf, obj, or ply file into the given Mesh instance.
  ///
  /// @param surface a Mesh instance representing a vertex-indexed tri-mesh. This will be filled.
  /// @param filename The path to the file from which to read the mesh. The format will be determined from the file extension as follows. File names ending with '.obj' are loaded as Wavefront OBJ files. File names ending with '.ply' are loaded as Stanford PLY files in format version 'ascii 1.0'. All other files are loaded as FreeSurfer binary surf files.
  /// @throws runtime_error if the file cannot be opened, domain_error if the surf file magic mismatches.  
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

  
  /// Determine the endianness of the system.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  bool _is_bigendian() {
    short int number = 0x1;
    char *numPtr = (char*)&number;
    return (numPtr[0] != 1);
  }  

  /// @brief Read per-vertex brain morphometry data from a FreeSurfer curv stream.
  /// @details The curv format is a simple binary format that stores one floating point value per vertex of a related brain surface.
  /// @param curv A Curv instance to be filled.
  /// @param is An open istream from which to read the curv data.
  /// @throws domain_error if the curv file magic mismatches or the curv file header claims that the file contains more than 1 value per vertex.
  void read_curv(Curv* curv, std::istream *is) {
    const int CURV_MAGIC = 16777215;
    int magic = _fread3(*is);
    if(magic != CURV_MAGIC) {
      throw std::domain_error("Magic did not match: expected " + std::to_string(CURV_MAGIC) + ", found " + std::to_string(magic) + ".\n");
    }
    curv->num_vertices = _freadt<int32_t>(*is);
    curv->num_faces =  _freadt<int32_t>(*is);
    curv->num_values_per_vertex = _freadt<int32_t>(*is);
    //std::cout << "Read file with " << num_verts << " vertices, " << num_faces << " faces and " << num_values_per_vertex << " values per vertex.\n";
    if(curv->num_values_per_vertex != 1) { // Not supported, I know no case where this is used. Please submit a PR with a demo file if you have one, and let me know where it came from.
      throw std::domain_error("Curv file must contain exactly 1 value per vertex, found " + std::to_string(curv->num_values_per_vertex) + ".\n");
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
  void read_curv(Curv* curv, const std::string& filename) {
    std::ifstream is(filename);
    if(is.is_open()) {
      read_curv(curv, &is);
      is.close();
    } else {
      throw std::runtime_error("Could not open curv file '" + filename + "' for reading.\n");
    }
  }

  /// Read an Annot Colortable from a stream.
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
  void read_annot(Annot* annot, const std::string& filename) {
    std::ifstream is(filename);
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
  std::vector<float> read_curv_data(const std::string& filename) {
    Curv curv;
    read_curv(&curv, filename);
    return(curv.data);
  }
  
  /// Swap endianness of a value.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
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
  void _fwritei3(std::ostream& os, uint32_t i) {
    unsigned char b1 = ( i >> 16) & 255;
    unsigned char b2 = ( i >> 8) & 255;
    unsigned char b3 =  i & 255;

    if(!_is_bigendian()) {
      b1 = _swap_endian<unsigned char>(b1);
      b2 = _swap_endian<unsigned char>(b2);
      b3 = _swap_endian<unsigned char>(b3);
      //std::cout << "Produced swapped BE values " << (int)b1 << "," << (int)b2 << "," << (int)b3 << ".\n";
    }
    
    os.write( reinterpret_cast<const char*>( &b1 ), sizeof(b1));
    os.write( reinterpret_cast<const char*>( &b2 ), sizeof(b2));
    os.write( reinterpret_cast<const char*>( &b3 ), sizeof(b3));
  }

  /// Read a '\n'-terminated ASCII string from a stream.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::string _freadstringnewline(std::istream &is) {
    std::string s;
    std::getline(is, s, '\n');
    return s;
  }

  /// Read a fixed length C-style string from an open binary stream. This does not care about trailing NULL bytes or anything, it just reads the given length of bytes.
  /// @throws std::out_of_range if length is not positive
  std::string _freadfixedlengthstring(std::istream &is, int32_t length, bool strip_last_char=true) {
    if(length <= 0) {
      throw std::out_of_range("Parameter 'length' must be a positive integer.\n");
    }
    std::string str;
    str.resize(length);
    is.read(&str[0], length);
    if(strip_last_char) {
      str = str.substr(0, length-1);
    }
    return str;
  }

  /// Check whether a string ends with the given suffix.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  /// https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
  bool _ends_with (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
  }


  /// @brief Write curv data to a stream.
  /// @details A curv file contains one floating point value per vertex (or a related mesh).
  /// @param os An output stream to which to write the data. The stream must be open, and this function will not close it after writing to it.
  /// @param curv_data the data to write.
  /// @param num_faces the value for the header field `num_faces`. This is not needed afaik and typically ignored.
  void write_curv(std::ostream& os, std::vector<float> curv_data, int32_t num_faces = 100000) {
    const uint32_t CURV_MAGIC = 16777215;
    _fwritei3(os, CURV_MAGIC);
    _fwritet<int32_t>(os, curv_data.size());
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
  struct Label {
    std::vector<int> vertex;
    std::vector<float> coord_x;
    std::vector<float> coord_y;
    std::vector<float> coord_z;
    std::vector<float> value;

    /// Compute for each vertex of the surface whether it is inside the label.
    std::vector<bool> vert_in_label(size_t surface_num_verts) const {
      if(surface_num_verts < this->vertex.size()) { // nonsense, so we warn (but don't throw, maybe the user really wants this).
        std::cerr << "Invalid number of vertices for surface, must be at least " << this->vertex.size() << "\n";
      }
      std::vector<bool> is_in;
      for(size_t i=0; i <surface_num_verts; i++) {
        is_in.push_back(false);
      }
      for(size_t i=0; i <this->vertex.size(); i++) {
        is_in[this->vertex[i]] = true;
      }
      return(is_in);
    }

    /// Return the number of entries (vertices/voxels) in this label.
    size_t num_entries() const {      
      size_t num_ent = this->vertex.size();
      if(this->coord_x.size() != num_ent || this->coord_y.size() != num_ent || this->coord_z.size() != num_ent || this->value.size() != num_ent || this->value.size() != num_ent) {
        std::cerr << "Inconsistent label: sizes of property vectors do not match.\n";
      }
      return(num_ent);
    }
  };


  /// @brief Read a FreeSurfer ASCII label from a stream.
  /// @details A label is a list of vertices (for a surface label, given by index) or voxels (for a volume label, given by the xyz coordinates) and one floating point value per vertex/voxel. Sometimes a label is only used to define a set of vertices/voxels (like a certain brain region), and the values are irrelevant (and typically left at 0.0).
  /// @param label A Label instance that should be filled.
  /// @param is An open stream from which to read the label.
  /// @see There exists an overload to read from a file instead.
  /// @throws std::domain_error if the label data format is incorrect
  void read_label(Label* label, std::ifstream* is) {
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
          //std::cout << "Line " << (line_idx+1) << ": vertex=" << vertex << ", x=" << x << ", y=" << y << ", z=" << z << ", value=" << value << ".\n";
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
  void read_label(Label* label, const std::string& filename) {
    std::ifstream infile(filename);
    if(infile.is_open()) {
      read_label(label, &infile);
      infile.close();
    } else {
      throw std::runtime_error("Could not open label file '" + filename + "' for reading.\n");
    }
  }


  /// Write label data to a stream.
  ///
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


  /// Write label data to a file.
  ///
  /// See also: swrite_label to write to a stream.
  /// @throws std::runtime_error if the file cannot be opened.
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

} // End namespace fs


