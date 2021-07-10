#pragma once

#include <iostream>
#include <climits>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <cassert>
#include <sstream>




namespace fs {
  
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
  int _fread3(std::istream& infile);
  template <typename T> T _freadt(std::istream& infile);
  std::string _freadstringnewline(std::istream& stream);
  bool _ends_with(std::string const &fullString, std::string const &ending);
  struct MghHeader;
  
  /// Models a triangular mesh, used for brain surface meshes. This is a vertex-indexed representation.
  ///
  /// Represents a vertex-indexed mesh. The n vertices are stored as 3D point coordinates (x,y,z) in a vector
  /// of length 3n, in which 3 consecutive values represent the x, y and z coordinate of the same vertex.
  /// The m faces are stored as a vector of 3m integers, where 3 consecutive values represent the 3 vertices (by index)
  /// making up the respective face. Vertex indices are 0-based.
  class Mesh {
    public:
      std::vector<float> vertices;
      std::vector<int> faces;

      /// Return string representing the mesh in Wavefront Object (.obj) format.
      std::string to_obj() {
        std::stringstream objs;
        for(size_t vidx=0; vidx<this->vertices.size();vidx+=3) { // vertex coords
          objs << "v " << vertices[vidx] << " " << vertices[vidx+1] << " " << vertices[vidx+2] << "\n";
        }
        for(size_t fidx=0; fidx<this->faces.size();fidx+=3) { // faces: vertex indices, 1-based
          objs << "f " << faces[fidx]+1 << " " << faces[fidx+1]+1 << " " << faces[fidx+2]+1 << "\n";
        }        
        return(objs.str());
      }

      /// Return the number of vertices in this mesh.
      size_t num_vertices() {
        return(this->vertices.size() / 3);
      }

      /// Return the number of faces in this mesh.
      size_t num_faces() {
        return(this->faces.size() / 3);
      }

      /// Return string representing the mesh in PLY format.
      std::string to_ply() {
        std::stringstream plys;
        plys << "ply\nformat ascii 1.0\n";
        plys << "element vertex " << this->num_vertices() << "\n";
        plys << "property float x\nproperty float y\nproperty float z\n";
        plys << "element face " << this->num_faces() << "\n";
        plys << "property list uchar int vertex_index\n";
        plys << "end_header\n";
        
        for(size_t vidx=0; vidx<this->vertices.size();vidx+=3) {  // vertex coords
          plys << vertices[vidx] << " " << vertices[vidx+1] << " " << vertices[vidx+2] << "\n";
        }

        const int num_vertices_per_face = 3;
        for(size_t fidx=0; fidx<this->faces.size();fidx+=3) { // faces: vertex indices, 0-based
          plys << num_vertices_per_face << " " << faces[fidx] << " " << faces[fidx+1] << " " << faces[fidx+2] << "\n";
        }        
        return(plys.str());
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

    float xsize;
    float ysize;
    float zsize;
    std::vector<float> Mdc;
    std::vector<float> Pxyz_c;
  };

  /// Models the data of an MGH file. Currently these are 1D vectors, but one can compute the 4D array using the dimXlength fields of the respective MghHeader.
  struct MghData {
    std::vector<int32_t> data_mri_int;
    std::vector<uint8_t> data_mri_uchar;
    std::vector<float> data_mri_float;
  };

  /// Models a whole MGH file.
  struct Mgh {
    MghHeader header;
    MghData data;    
  };

  /// A simple 4D array datastructure, useful for representing volume data.
  ///
  /// Based on https://stackoverflow.com/questions/33113403/store-a-4d-array-in-a-vector
  template<class T> 
  struct Array4D {
    Array4D(unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4) :
      d1(d1), d2(d2), d3(d3), d4(d4), data(d1*d2*d3*d4) {}

    Array4D(MghHeader *mgh_header) :
      d1(mgh_header->dim1length), d2(mgh_header->dim2length), d3(mgh_header->dim3length), d4(mgh_header->dim4length), data(d1*d2*d3*d4) {}

    Array4D(Mgh *mgh) : // This does NOT init the data atm.
      d1(mgh->header.dim1length), d2(mgh->header.dim2length), d3(mgh->header.dim3length), d4(mgh->header.dim4length), data(d1*d2*d3*d4) {}
  
    /// Get the value at the given 4D position.
    T& at(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4) {
      return data[get_index(i1, i2, i3, i4)];
    }
  
    /// Get the index in the vector for the given 4D position.
    unsigned int get_index(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4) {
      assert(i1 >= 0 && i1 < d1);
      assert(i2 >= 0 && i2 < d2);
      assert(i3 >= 0 && i3 < d3);
      assert(i4 >= 0 && i4 < d4);
      return (((i1*d2 + i2)*d3 + i3)*d4 + i4);
    }
  
    unsigned int d1;
    unsigned int d2;
    unsigned int d3;
    unsigned int d4;
    std::vector<T> data;
  };

  // More declarations, should also go to separate header.
  void read_mgh_header(MghHeader* mgh_header, std::string filename);
  template <typename T> std::vector<T> _read_mgh_data(MghHeader*, std::string);
  std::vector<int32_t> _read_mgh_data_int(MghHeader*, std::string);
  std::vector<uint8_t> _read_mgh_data_uchar(MghHeader*, std::string);
  std::vector<float> _read_mgh_data_float(MghHeader*, std::string);


  /// Read a FreeSurfer volume file in MGH format into the given Mgh struct.
  void read_mgh(Mgh* mgh, std::string filename) {
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
    } else {      
      std::cout << "Not reading MGH data from file '" << filename << "', data type " << mgh->header.dtype << " not supported yet.\n";
      if(_ends_with(filename, ".mgz")) {
        std::cout << "Note: your MGH filename ends with '.mgz'. Keep in mind that MGZ format is not supported yet.\n";  
      }
      exit(1);
    }
  }

  /// Read the header of a FreeSurfer volume file in MGH format into the given MghHeader struct.
  void read_mgh_header(MghHeader* mgh_header, std::string filename) {
    const int MGH_VERSION = 1;    

    std::ifstream infile;
    infile.open(filename, std::ios_base::in | std::ios::binary);
    if(infile.is_open()) {
      int format_version = _freadt<int32_t>(infile);
      if(format_version != MGH_VERSION) {        
        std::cerr << "Invalid MGH file or unsupported file format version: expected version " << MGH_VERSION << ", found " << format_version << ".\n";
        if(_ends_with(filename, ".mgz")) {
          std::cout << "Note: your MGH filename ends with '.mgz'. Keep in mind that MGZ format is not supported yet.\n";  
        }
        exit(1);
      }
      mgh_header->dim1length =  _freadt<int32_t>(infile);
      mgh_header->dim2length =  _freadt<int32_t>(infile);
      mgh_header->dim3length =  _freadt<int32_t>(infile);
      mgh_header->dim4length =  _freadt<int32_t>(infile);

      mgh_header->dtype =  _freadt<int32_t>(infile);
      mgh_header->dof =  _freadt<int32_t>(infile);

      int unused_header_space_size_left = 256;  // in bytes
      mgh_header->ras_good_flag =  _freadt<int16_t>(infile);
      unused_header_space_size_left -= 2; // for the ras_good_flag

      // Read the RAS part of the header.
      if(mgh_header->ras_good_flag == 1) {
        mgh_header->xsize =  _freadt<_Float32>(infile);
        mgh_header->ysize =  _freadt<_Float32>(infile);
        mgh_header->zsize =  _freadt<_Float32>(infile);

        for(int i=0; i<9; i++) {
          mgh_header->Mdc.push_back( _freadt<_Float32>(infile));
        }
        for(int i=0; i<3; i++) {
          mgh_header->Pxyz_c.push_back( _freadt<_Float32>(infile));
        }
        unused_header_space_size_left -= 60;
      }
      // TODO: MGH files may contain an optional footer after the data with some metadata that we could read.
      //infile.seekg(unused_header_space_size_left, infile.cur);  // skip rest of header.
      // skip past data
      // check if something left in file and read footer if existant

      infile.close();
    } else {
      std::cerr << "Unable to open MGH file '" << filename << "'.\n";
      exit(1);
    }
  }

  /// Read MRI_INT data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<int32_t> _read_mgh_data_int(MghHeader* mgh_header, std::string filename) {
    if(mgh_header->dtype != MRI_INT) {
      std::cerr << "Expected MRI data type " << MRI_INT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<int32_t>(mgh_header, filename));
  }

  /// Read arbitrary MGH data.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  template <typename T>
  std::vector<T> _read_mgh_data(MghHeader* mgh_header, std::string filename) {
    std::ifstream infile;
    infile.open(filename, std::ios_base::in | std::ios::binary);
    if(infile.is_open()) {
      infile.seekg(284, infile.beg); // skip to end of header and beginning of data

      int num_values = mgh_header->dim1length * mgh_header->dim2length * mgh_header->dim3length * mgh_header->dim4length;
      std::vector<T> data;
      for(int i=0; i<num_values; i++) {
        data.push_back( _freadt<T>(infile));
      }
      infile.close();
      return(data);
    } else {
      std::cerr << "Unable to open MGH file '" << filename << "'.\n";
      exit(1);
    }
  }

  /// Read MRI_FLOAT data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<_Float32> _read_mgh_data_float(MghHeader* mgh_header, std::string filename) {
    if(mgh_header->dtype != MRI_FLOAT) {
      std::cerr << "Expected MRI data type " << MRI_FLOAT << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<_Float32>(mgh_header, filename));
  }

  /// Read MRI_UCHAR data from MGH file
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<uint8_t> _read_mgh_data_uchar(MghHeader* mgh_header, std::string filename) {
    if(mgh_header->dtype != MRI_UCHAR) {
      std::cerr << "Expected MRI data type " << MRI_UCHAR << ", but found " << mgh_header->dtype << ".\n";
    }
    return(_read_mgh_data<uint8_t>(mgh_header, filename));
  }

  /// Read a brain mesh from a file in binary FreeSurfer 'surf' format into the given Mesh instance.
  ///
  /// Returns a Mesh datastructure representing a vertex-indexed tri-mesh.
  void read_surf(Mesh* surface, std::string filename) {
    const int SURF_TRIS_MAGIC = 16777214;
    std::ifstream infile;
    infile.open(filename, std::ios_base::in | std::ios::binary);
    if(infile.is_open()) {
      int magic = _fread3(infile);
      if(magic != SURF_TRIS_MAGIC) {
        std::cerr << "Magic did not match: expected " << SURF_TRIS_MAGIC << ", found " << magic << ".\n";
        exit(1);
      }
      std::string created_line = _freadstringnewline(infile);
      std::string comment_line = _freadstringnewline(infile);
      int num_verts =  _freadt<int32_t>(infile);
      int num_faces =  _freadt<int32_t>(infile);      
      //std::cout << "Read surface file with " << num_verts << " vertices, " << num_faces << " faces.\n";
      std::vector<float> vdata;
      for(int i=0; i<(num_verts*3); i++) {
        vdata.push_back( _freadt<_Float32>(infile));
      }
      std::vector<int> fdata;
      for(int i=0; i<(num_faces*3); i++) {
        fdata.push_back( _freadt<int32_t>(infile));
      }
      infile.close();
      surface->vertices = vdata;
      surface->faces = fdata;
    } else {
      std::cerr << "Unable to open surface file '" << filename << "'.\n";
      exit(1);
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

  /// Read per-vertex brain morphometry data from a FreeSurfer curv format file.
  std::vector<float> read_curv(std::string filename) {
    const int CURV_MAGIC = 16777215;
    std::ifstream infile;
    infile.open(filename, std::ios_base::in | std::ios::binary);
    if(infile.is_open()) {
      int magic = _fread3(infile);
      if(magic != CURV_MAGIC) {
        std::cerr << "Magic did not match: expected " << CURV_MAGIC << ", found " << magic << ".\n";
      }
      int num_verts =  _freadt<int32_t>(infile);
      int num_faces =  _freadt<int32_t>(infile);
      (void)num_faces; // The num_faces it unused but needs to be read, we perform a no-op here to aviodi a compiler warning about an 'unused variable'.
      int num_values_per_vertex =  _freadt<int32_t>(infile);
      //std::cout << "Read file with " << num_verts << " vertices, " << num_faces << " faces and " << num_values_per_vertex << " values per vertex.\n";
      if(num_values_per_vertex != 1) { // Not supported, I know no case where this is used. Please submit a PR with a demo file if you have one, and let me know where it came from.
        std::cerr << "Curv file must contain exactly 1 value per vertex, found " << num_values_per_vertex << ".\n";  
      }
      std::vector<float> data;
      for(int i=0; i<num_verts; i++) {
        data.push_back( _freadt<_Float32>(infile));
      }
      infile.close();
      return(data);
    } else {
      std::cerr << "Unable to open curvature file '" << filename << "'.\n";
      exit(1);
    }
    
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

  /// Read a big endian value from a file.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  template <typename T>
  T _freadt(std::istream& infile) {
    T t;
    infile.read(reinterpret_cast<char*>(&t), sizeof(t));
    if(! _is_bigendian()) {
      t = _swap_endian<T>(t);
    }
    return(t);  
  }

  /// Read 3 big endian bytes as a single integer from a stream.
  ///
  /// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  int _fread3(std::istream& infile) {
    uint32_t i;
    infile.read(reinterpret_cast<char*>(&i), 3);
    if(! _is_bigendian()) {
      i = _swap_endian<std::uint32_t>(i);
    }
    i = ((i >> 8) & 0xffffff);
    return(i);
  }

  /// Write a value to a file as big endian.
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
  std::string _freadstringnewline(std::istream &stream) {
    std::string s;
    std::getline(stream, s, '\n');
    return s;
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

  /// Write curv data to a stream. The stream must be open.
  void swrite_curv(std::ostream& os, std::vector<float> curv_data, int32_t num_faces = 100000) {
    const uint32_t CURV_MAGIC = 16777215;
    _fwritei3(os, CURV_MAGIC);
     _fwritet<int32_t>(os, curv_data.size());
     _fwritet<int32_t>(os, num_faces);
     _fwritet<int32_t>(os, 1); // Number of values per vertex.
    for(size_t i=0; i<curv_data.size(); i++) {
       _fwritet<_Float32>(os, curv_data[i]);
    }
  }

  /// Write curv data to a file.
  ///
  /// See also: swrite_curv to write to a stream.
  void write_curv(std::string filename, std::vector<float> curv_data, int32_t num_faces = 100000) {
    std::ofstream ofs;
    ofs.open(filename, std::ofstream::out | std::ofstream::binary);
    if(ofs.is_open()) {
      swrite_curv(ofs, curv_data, num_faces);
      ofs.close();
    } else {
      std::cerr << "Unable to open curvature file '" << filename << "' for writing.\n";
      exit(1);
    }
  }

  /// Write MGH data to a stream. The stream must be open.
  void swrite_mgh(std::ostream& os, const Mgh& mgh) {
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
       _fwritet<_Float32>(os, mgh.header.xsize);
       _fwritet<_Float32>(os, mgh.header.ysize);
       _fwritet<_Float32>(os, mgh.header.zsize);

      for(int i=0; i<9; i++) {
         _fwritet<_Float32>(os, mgh.header.Mdc[i]);
      }
      for(int i=0; i<3; i++) {
         _fwritet<_Float32>(os, mgh.header.Pxyz_c[i]);
      }

      unused_header_space_size_left -= 60;
    }

    for(size_t i=0; i<unused_header_space_size_left; i++) {  // Fill rest of header space.
       _fwritet<uint8_t>(os, 0);
    }

    // Write data
    size_t num_values = mgh.header.dim1length * mgh.header.dim2length * mgh.header.dim3length * mgh.header.dim4length;
    if(mgh.header.dtype == MRI_INT) {
      if(mgh.data.data_mri_int.size() != num_values) {
        std::cerr << "Detected mismatch of MRI_INT data size and MGH header dim length values.\n";
        exit(1);
      }
      for(size_t i=0; i<num_values; i++) {
         _fwritet<int32_t>(os, mgh.data.data_mri_int[i]);
      }
    } else if(mgh.header.dtype == MRI_FLOAT) {
      if(mgh.data.data_mri_float.size() != num_values) {
        std::cerr << "Detected mismatch of MRI_FLOAT data size and MGH header dim length values.\n";
        exit(1);
      }
      for(size_t i=0; i<num_values; i++) {
         _fwritet<_Float32>(os, mgh.data.data_mri_float[i]);
      }
    } else if(mgh.header.dtype == MRI_UCHAR) {
      if(mgh.data.data_mri_uchar.size() != num_values) {
        std::cerr << "Detected mismatch of MRI_UCHAR data size and MGH header dim length values.\n";
        exit(1);
      }
      for(size_t i=0; i<num_values; i++) {
         _fwritet<uint8_t>(os, mgh.data.data_mri_uchar[i]);
      }
    } else {
      std::cerr << "Unsupported MRI data type " << mgh.header.dtype << ", cannot write MGH data.\n";
      exit(1);
    }
    
  }

  /// Write MGH data to a file.
  ///
  /// See also: swrite_mgh to write to a stream.
  void write_mgh(std::string filename, const Mgh& mgh) {
    std::ofstream ofs;
    ofs.open(filename, std::ofstream::out | std::ofstream::binary);
    if(ofs.is_open()) {
      swrite_mgh(ofs, mgh);
      ofs.close();
    } else {
      std::cerr << "Unable to open MGH file '" << filename << "' for writing.\n";
      exit(1);
    }
  }

  /// Models a FreeSurfer label.
  struct Label {
    std::vector<int> vertex;
    std::vector<float> coord_x;
    std::vector<float> coord_y;
    std::vector<float> coord_z;
    std::vector<float> value;

    // Compute for each vertex of the surface whether it is inside the label.
    std::vector<bool> vert_in_label(size_t surface_num_verts) {
      if(surface_num_verts < this->vertex.size()) { // nonsense, so we warn (but don't exit, maybe the user really wants this).
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
  };

  /// Read a FreeSurfer ASCII label file.
  ///
  /// Examples:
  ///
  /// sf::Label label;
  /// sf::read_label(&label, "subject1/label/lh.cortex.label");
  /// size_t nv = label.vertex.size();
  void read_label(Label* label, std::string filename) {
    std::ifstream infile(filename);
    if(infile.is_open()) {
      std::string line;
      int line_idx = -1;
      size_t num_entries_header = 0;  // number of vertices/voxels according to header
      size_t num_entries = 0;  // number of vertices/voxels for which the file contains label entries.
      while (std::getline(infile, line)) {
        line_idx += 1;
        std::istringstream iss(line);
        if(line_idx == 0) {
          continue; // skip comment.
        } else {
          if(line_idx == 1) {
            if (!(iss >> num_entries_header)) { 
              std::cerr << "Could not parse entry count from label file, invalid file.\n";
              exit(1);
            } 
          } else {
            int vertex; float x, y, z, value;
            if (!(iss >> vertex >> x >> y >> z >> value)) { 
              std::cerr << "Could not parse line " << (line_idx+1) << " of label file, invalid file.\n";
              exit(1);
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
        std::cerr << "Expected " << num_entries_header << " entries from label file header, but found " << num_entries << " in file, invalid label file.\n";
        exit(1);
      }
      if(label->vertex.size() != num_entries || label->coord_x.size() != num_entries || label->coord_y.size() != num_entries || label->coord_z.size() != num_entries || label->value.size() != num_entries) {
        std::cerr << "Expected " << num_entries << " entries in all Label vectors, but some did not match.\n";
      }
    } else {
      std::cerr << "Could not open label file for reading.\n";
      exit(1);
    }
  }


} // End namespace fs

