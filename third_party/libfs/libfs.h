#pragma once

#include <iostream>
#include <climits>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <cassert>
#include <sstream>




namespace fs {
  
  const int MRI_UCHAR = 0; // MRI data types, used by the MGH functions.
  const int MRI_INT = 1;
  const int MRI_FLOAT = 3; 
  const int MRI_SHORT = 4;

  // Declarations, show go to a header.
  int fread3(std::istream& infile);
  float freadf4(std::istream& infile);
  int freadi32(std::istream& infile);
  int freadi16(std::istream& infile);
  uint8_t freadu8(std::istream& infile);
  std::string freadstringzero(std::istream& stream);
  std::string freadstringnewline(std::istream& stream);
  bool ends_with(std::string const &fullString, std::string const &ending);
  struct MghHeader;
  
  // Models a triangular mesh, used for brain surface meshes. This is a vertex-indexed representation.
  class Mesh {
    public:
      std::vector<float> vertices;
      std::vector<int> faces;
  };  

  // Models the header of an MGH file.
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

  // Models the data of an MGH file. Currently these are 1D vectors, but one can compute the 4D array using the dimXlength fields of the respective MghHeader.
  struct MghData {
    std::vector<int32_t> data_mri_int;
    std::vector<uint8_t> data_mri_uchar;
    std::vector<float> data_mri_float;
  };

  // Models a whole MGH file.
  struct Mgh {
    MghHeader header;
    MghData data;    
  };

  // A simple 4D array datastructure. Needs to be templated. Useful for representing volume data.
  // Based on https://stackoverflow.com/questions/33113403/store-a-4d-array-in-a-vector
  template<class T> 
  struct Array4D {
    Array4D(unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4) :
      d1(d1), d2(d2), d3(d3), d4(d4), data(d1*d2*d3*d4) {}

    Array4D(MghHeader *mgh_header) :
      d1(mgh_header->dim1length), d2(mgh_header->dim2length), d3(mgh_header->dim3length), d4(mgh_header->dim4length), data(d1*d2*d3*d4) {}

    Array4D(Mgh *mgh) : // This does NOT init the data atm.
      d1(mgh->header.dim1length), d2(mgh->header.dim2length), d3(mgh->header.dim3length), d4(mgh->header.dim4length), data(d1*d2*d3*d4) {}
  
    T& at(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4)
    {
      return data[getIndex(i1, i2, i3, i4)];
    }
  
    unsigned int getIndex(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4)
    {
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
  std::vector<int32_t> read_mgh_data_int(MghHeader*, std::string);
  std::vector<uint8_t> read_mgh_data_uchar(MghHeader*, std::string);
  std::vector<float> read_mgh_data_float(MghHeader*, std::string);


  // Read a FreeSurfer volume file in MGH format into the given Mgh struct.
  void read_mgh(Mgh* mgh, std::string filename) {
    MghHeader mgh_header;
    read_mgh_header(&mgh_header, filename);
    mgh->header = mgh_header;
    if(mgh->header.dtype == MRI_INT) {
      std::vector<int32_t> data = read_mgh_data_int(&mgh_header, filename);
      mgh->data.data_mri_int = data;
    } else if(mgh->header.dtype == MRI_UCHAR) {
      std::vector<uint8_t> data = read_mgh_data_uchar(&mgh_header, filename);
      mgh->data.data_mri_uchar = data;
    } else if(mgh->header.dtype == MRI_FLOAT) {
      std::vector<float> data = read_mgh_data_float(&mgh_header, filename);
      mgh->data.data_mri_float = data;
    } else {      
      std::cout << "Not reading MGH data from file '" << filename << "', data type " << mgh->header.dtype << " not supported yet.\n";
      if(ends_with(filename, ".mgz")) {
        std::cout << "Note: your MGH filename ends with '.mgz'. Keep in mind that MGZ format is not supported yet.\n";  
      }
      exit(1);
    }
  }

  // Read the header of a FreeSurfer volume file in MGH format into the given MghHeader struct.
  void read_mgh_header(MghHeader* mgh_header, std::string filename) {
    const int MGH_VERSION = 1;    

    std::ifstream infile;
    infile.open(filename, std::ios_base::in | std::ios::binary);
    if(infile.is_open()) {
      int format_version = freadi32(infile);
      if(format_version != MGH_VERSION) {        
        std::cerr << "Invalid MGH file or unsupported file format version: expected version " << MGH_VERSION << ", found " << format_version << ".\n";
        if(ends_with(filename, ".mgz")) {
          std::cout << "Note: your MGH filename ends with '.mgz'. Keep in mind that MGZ format is not supported yet.\n";  
        }
        exit(1);
      }
      mgh_header->dim1length = freadi32(infile);
      mgh_header->dim2length = freadi32(infile);
      mgh_header->dim3length = freadi32(infile);
      mgh_header->dim4length = freadi32(infile);

      mgh_header->dtype = freadi32(infile);
      mgh_header->dof = freadi32(infile);

      int unused_header_space_size_left = 256;  // in bytes
      mgh_header->ras_good_flag = freadi16(infile);
      unused_header_space_size_left -= 2; // for the ras_good_flag

      // Read the RAS part of the header.
      if(mgh_header->ras_good_flag == 1) {
        mgh_header->xsize = freadf4(infile);
        mgh_header->ysize = freadf4(infile);
        mgh_header->zsize = freadf4(infile);

        for(int i=0; i<9; i++) {
          mgh_header->Mdc.push_back(freadf4(infile));
        }
        for(int i=0; i<3; i++) {
          mgh_header->Pxyz_c.push_back(freadf4(infile));
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

  // Read MRI_INT data from MGH file
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<int32_t> read_mgh_data_int(MghHeader* mgh_header, std::string filename) {
    std::ifstream infile;
    infile.open(filename, std::ios_base::in | std::ios::binary);
    if(mgh_header->dtype != 1) {
      std::cerr << "Expected MRI data type 1, but found " << mgh_header->dtype << ".\n";
    }
    if(infile.is_open()) {
      infile.seekg(284, infile.beg); // skip to end of header and beginning of data

      int num_values = mgh_header->dim1length * mgh_header->dim2length * mgh_header->dim3length * mgh_header->dim4length;
      std::vector<int32_t> data;
      for(int i=0; i<num_values; i++) {
        data.push_back(freadi32(infile));
      }
      infile.close();
      return(data);
    } else {
      std::cerr << "Unable to open MGH file '" << filename << "'.\n";
      exit(1);
    }
  }

  // Read MRI_FLOAT data from MGH file
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<float> read_mgh_data_float(MghHeader* mgh_header, std::string filename) {
    std::ifstream infile;
    infile.open(filename, std::ios_base::in | std::ios::binary);
    if(mgh_header->dtype != 3) {
      std::cerr << "Expected MRI data type 3, but found " << mgh_header->dtype << ".\n";
    }
    if(infile.is_open()) {
      infile.seekg(284, infile.beg); // skip to end of header and beginning of data

      int num_values = mgh_header->dim1length * mgh_header->dim2length * mgh_header->dim3length * mgh_header->dim4length;
      std::vector<float> data;
      for(int i=0; i<num_values; i++) {
        data.push_back(freadf4(infile));
      }
      infile.close();
      return(data);
    } else {
      std::cerr << "Unable to open MGH file '" << filename << "'.\n";
      exit(1);
    }
  }

  // Read MRI_UCHAR data from MGH file
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::vector<uint8_t> read_mgh_data_uchar(MghHeader* mgh_header, std::string filename) {
    std::ifstream infile;
    infile.open(filename, std::ios_base::in | std::ios::binary);
    if(mgh_header->dtype != 0) {
      std::cerr << "Expected MRI data type 0, but found " << mgh_header->dtype << ".\n";
    }
    if(infile.is_open()) {
      infile.seekg(284, infile.beg); // skip to end of header and beginning of data

      int num_values = mgh_header->dim1length * mgh_header->dim2length * mgh_header->dim3length * mgh_header->dim4length;
      std::vector<uint8_t> data;
      for(int i=0; i<num_values; i++) {
        data.push_back(freadu8(infile));
      }
      infile.close();
      return(data);
    } else {
      std::cerr << "Unable to open MGH file '" << filename << "'.\n";
      exit(1);
    }
  }

  // Read a brain mesh from a file in binary FreeSurfer 'surf' format into the given Mesh instance.
  void read_surf(Mesh* surface, std::string filename) {
    const int SURF_TRIS_MAGIC = 16777214;
    std::ifstream infile;
    infile.open(filename, std::ios_base::in | std::ios::binary);
    if(infile.is_open()) {
      int magic = fread3(infile);
      if(magic != SURF_TRIS_MAGIC) {
        std::cerr << "Magic did not match: expected " << SURF_TRIS_MAGIC << ", found " << magic << ".\n";
        exit(1);
      }
      std::string created_line = freadstringnewline(infile);
      std::string comment_line = freadstringnewline(infile);
      int num_verts = freadi32(infile);
      int num_faces = freadi32(infile);      
      //std::cout << "Read surface file with " << num_verts << " vertices, " << num_faces << " faces.\n";
      std::vector<float> vdata;
      for(int i=0; i<(num_verts*3); i++) {
        vdata.push_back(freadf4(infile));
      }
      std::vector<int> fdata;
      for(int i=0; i<(num_faces*3); i++) {
        fdata.push_back(freadi32(infile));
      }
      infile.close();
      surface->vertices = vdata;
      surface->faces = fdata;
    } else {
      std::cerr << "Unable to open surface file '" << filename << "'.\n";
      exit(1);
    }
  }

  
  // Determine the endianness of the system.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  bool is_bigendian() {
    short int number = 0x1;
    char *numPtr = (char*)&number;
    return (numPtr[0] != 1);
  }

  // Read per-vertex brain morphometry data from a FreeSurfer curv format file.
  std::vector<float> read_curv(std::string filename) {
    const int CURV_MAGIC = 16777215;
    std::ifstream infile;
    infile.open(filename, std::ios_base::in | std::ios::binary);
    if(infile.is_open()) {
      int magic = fread3(infile);
      if(magic != CURV_MAGIC) {
        std::cerr << "Magic did not match: expected " << CURV_MAGIC << ", found " << magic << ".\n";
      }
      int num_verts = freadi32(infile);
      int num_faces = freadi32(infile);
      (void)num_faces; // The num_faces it unused but needs to be read, we perform a no-op here to aviodi a compiler warning about an 'unused variable'.
      int num_values_per_vertex = freadi32(infile);
      //std::cout << "Read file with " << num_verts << " vertices, " << num_faces << " faces and " << num_values_per_vertex << " values per vertex.\n";
      if(num_values_per_vertex != 1) { // Not supported, I know no case where this is used. Please submit a PR with a demo file if you have one, and let me know where it came from.
        std::cerr << "Curv file must contain exactly 1 value per vertex, found " << num_values_per_vertex << ".\n";  
      }
      std::vector<float> data;
      for(int i=0; i<num_verts; i++) {
        data.push_back(freadf4(infile));
      }
      infile.close();
      return(data);
    } else {
      std::cerr << "Unable to open curvature file '" << filename << "'.\n";
      exit(1);
    }
    
  }
  
  // Swap endianness of a value.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  template <typename T>
  T swap_endian(T u) {
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

  // Read a single big endian 32 bit integer from a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  int freadi32(std::istream& infile) {
    int32_t i;
    infile.read(reinterpret_cast<char*>(&i), sizeof(i));
    if(! is_bigendian()) {
      i = swap_endian<std::int32_t>(i);
    }
    return(i);
  }


  // Read a single big endian 16 bit integer from a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  int freadi16(std::istream& infile) {
    int16_t i;
    infile.read(reinterpret_cast<char*>(&i), sizeof(i));
    if(! is_bigendian()) {
      i = swap_endian<std::int16_t>(i);
    }
    return(i);
  }

  // Read a single big endian uint8 from a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  uint8_t freadu8(std::istream& infile) {
    uint8_t i;
    infile.read(reinterpret_cast<char*>(&i), sizeof(i));
    //std::cout << "Read raw uint8_t " << (unsigned int)i << ".\n";
    
    if(! is_bigendian()) {
      i = swap_endian<std::uint8_t>(i);
    }
    //std::cout << " -Produced int " << (int)i << "\n";
    return(i);
  }

  // Read 3 big endian bytes as a single integer from a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  int fread3(std::istream& infile) {
    uint32_t i;
    infile.read(reinterpret_cast<char*>(&i), 3);
    if(! is_bigendian()) {
      i = swap_endian<std::uint32_t>(i);
    }
    i = ((i >> 8) & 0xffffff);
    return(i);
  }

  // Read big endian 4 byte float from a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  float freadf4(std::istream& infile) {
    _Float32 f;
    infile.read(reinterpret_cast<char*>(&f), sizeof(f));
    if(! is_bigendian()) {
      f = swap_endian<_Float32>(f);
    }
    return(f);
  }

  // Write big endian 4 byte float to a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  void fwritef4(std::ostream& os, _Float32 f) {
    if(! is_bigendian()) {
      f = swap_endian<_Float32>(f);
    }
    os.write( reinterpret_cast<const char*>( &f ), sizeof(f));
  }

  // Write big endian 32 bit integer to a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  void fwritei32(std::ostream& os, int32_t i) {
    if(! is_bigendian()) {
      i = swap_endian<int32_t>(i);
    }
    os.write( reinterpret_cast<const char*>( &i ), sizeof(i));
  }

  // Write big endian 8 bit unsigned integer to a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  void fwriteu8(std::ostream& os, uint8_t i) {
    if(! is_bigendian()) {
      i = swap_endian<uint8_t>(i);
    }
    os.write( reinterpret_cast<const char*>( &i ), sizeof(i));
  }

  // Write big endian 16 bit integer to a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  void fwritei16(std::ostream& os, int16_t i) {
    if(! is_bigendian()) {
      i = swap_endian<int16_t>(i);
    }
    os.write( reinterpret_cast<const char*>( &i ), sizeof(i));
  }

  // Write big endian 24 bit integer to a stream, extracted from the first 3 bytes of an unsigned 32 bit integer.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  void fwritei3(std::ostream& os, uint32_t i) {
    unsigned char b1 = ( i >> 16) & 255;
    unsigned char b2 = ( i >> 8) & 255;
    unsigned char b3 =  i & 255;

    if(! is_bigendian()) {
      b1 = swap_endian<unsigned char>(b1);
      b2 = swap_endian<unsigned char>(b2);
      b3 = swap_endian<unsigned char>(b3);
      //std::cout << "Produced swapped BE values " << (int)b1 << "," << (int)b2 << "," << (int)b3 << ".\n";
    }
    
    os.write( reinterpret_cast<const char*>( &b1 ), sizeof(b1));
    os.write( reinterpret_cast<const char*>( &b2 ), sizeof(b2));
    os.write( reinterpret_cast<const char*>( &b3 ), sizeof(b3));
  }

  // Read a C-style zero-terminated ASCII string from a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::string freadstringzero(std::istream &stream) {
    std::string s;
    std::getline(stream, s, '\0');
    return s;
  }

  // Read a '\n'-terminated ASCII string from a stream.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  std::string freadstringnewline(std::istream &stream) {
    std::string s;
    std::getline(stream, s, '\n');
    return s;
  }

  // Check whether a string ends with the given suffix.
  //
  // THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
  // https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
  bool ends_with (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
  }

  // Write curv data to a stream. The stream must be open.
  void swrite_curv(std::ostream& os, std::vector<float> curv_data, int32_t num_faces = 100000) {
    const uint32_t CURV_MAGIC = 16777215;
    fwritei3(os, CURV_MAGIC);
    fwritei32(os, curv_data.size());
    fwritei32(os, num_faces);
    fwritei32(os, 1); // Number of values per vertex.
    for(size_t i=0; i<curv_data.size(); i++) {
      fwritef4(os, curv_data[i]);
    }
  }

  // Write curv data to a file.
  //
  // See also: swrite_curv to write to a stream.
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

  // Write MGH data to a stream. The stream must be open.
  void swrite_mgh(std::ostream& os, const Mgh& mgh) {
    fwritei32(os, 1); // MGH file format version
    fwritei32(os, mgh.header.dim1length);
    fwritei32(os, mgh.header.dim2length);
    fwritei32(os, mgh.header.dim3length);
    fwritei32(os, mgh.header.dim4length);

    fwritei32(os, mgh.header.dtype);
    fwritei32(os, mgh.header.dof);

    size_t unused_header_space_size_left = 256;  // in bytes
    fwritei16(os, mgh.header.ras_good_flag);
    unused_header_space_size_left -= 2; // for RAS flag

    // Write RAS part of of header if flag is 1.
    if(mgh.header.ras_good_flag == 1) {
      fwritef4(os, mgh.header.xsize);
      fwritef4(os, mgh.header.ysize);
      fwritef4(os, mgh.header.zsize);

      for(int i=0; i<9; i++) {
        fwritef4(os, mgh.header.Mdc[i]);
      }
      for(int i=0; i<3; i++) {
        fwritef4(os, mgh.header.Pxyz_c[i]);
      }

      unused_header_space_size_left -= 60;
    }

    for(size_t i=0; i<unused_header_space_size_left; i++) {  // Fill rest of header space.
      fwriteu8(os, 0);
    }

    // Write data
    size_t num_values = mgh.header.dim1length * mgh.header.dim2length * mgh.header.dim3length * mgh.header.dim4length;
    if(mgh.header.dtype == MRI_INT) {
      if(mgh.data.data_mri_int.size() != num_values) {
        std::cerr << "Detected mismatch of MRI_INT data size and MGH header dim length values.\n";
        exit(1);
      }
      for(size_t i=0; i<num_values; i++) {
        fwritei32(os, mgh.data.data_mri_int[i]);
      }
    } else if(mgh.header.dtype == MRI_FLOAT) {
      if(mgh.data.data_mri_float.size() != num_values) {
        std::cerr << "Detected mismatch of MRI_FLOAT data size and MGH header dim length values.\n";
        exit(1);
      }
      for(size_t i=0; i<num_values; i++) {
        fwritef4(os, mgh.data.data_mri_float[i]);
      }
    } else if(mgh.header.dtype == MRI_UCHAR) {
      if(mgh.data.data_mri_uchar.size() != num_values) {
        std::cerr << "Detected mismatch of MRI_UCHAR data size and MGH header dim length values.\n";
        exit(1);
      }
      for(size_t i=0; i<num_values; i++) {
        fwriteu8(os, mgh.data.data_mri_uchar[i]);
      }
    } else {
      std::cerr << "Unsupported MRI data type " << mgh.header.dtype << ", cannot write MGH data.\n";
      exit(1);
    }
    
  }

  // Write MGH data to a file.
  //
  // See also: swrite_mgh to write to a stream.
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

  // Models a FreeSurfer label.
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

  // Read a FreeSurfer ASCII label file.
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

