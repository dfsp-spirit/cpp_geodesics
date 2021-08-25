#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

/// Determine the endianness of the system.
///
/// THIS FUNCTION IS INTERNAL AND SHOULD NOT BE CALLED BY API CLIENTS.
bool _is_bigendian() {
    short int number = 0x1;
    char *numPtr = (char*)&number;
    return (numPtr[0] != 1);
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

template <typename T>
int32_t _vv_data_type_code() { return(14); }

template <int32_t>
int32_t _vv_data_type_code() { return(13); }

/// Write vector of vectors to binary big endian file.
/// The format is: 2 magic int32 numbers followed by size of outer vec (also as int32). Then, for each inner vec: int32 size of vec, then the values.
template <typename T>
void write_vv(const std::string& filename, std::vector<std::vector<T>> data) {
    std::ofstream ofs;
    ofs.open(filename, std::ofstream::out | std::ofstream::binary);
    if(ofs.is_open()) {
        _fwritet<int32_t>(ofs, 42); // write magic number
        _fwritet<int32_t>(ofs, _vv_data_type_code<T>()); // write data-type code. 13=int_32, 14=float_32.
        _fwritet<int32_t>(ofs, data.size());
        for(size_t i=0; i<data.size(); i++) {
            _fwritet<int32_t>(ofs, data[i].size());
            for(size_t j=0; j<data[i].size(); j++) {
                _fwritet<T>(ofs, data[i][j]);
            }
        }
      ofs.close();
    } else {
      throw std::runtime_error("Unable to open file '" + filename + "' for writing.\n");
    }
  }