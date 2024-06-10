#pragma once

#include "npy.hpp"


template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T>>& v) {
    std::size_t total_size = 0;
    for (const auto& sub : v) {
        total_size += sub.size();
    }
    std::vector<T> result;
    result.reserve(total_size);
    for (const auto& sub : v) {
        result.insert(result.end(), sub.begin(), sub.end());
    }
    return result;
}

/// @brief  Write array of array flattened into 2 numpy files. One flattened data file, and one header file with type int32_t that contains the length of the individual arrays in the main array.
/// @tparam T the type, typically int32_t or float32_t
/// @param filename string, the output file to write. A second file with ".hdr" suffix will be written that contains the header information: the length of the individual sub arrays that were flattened to the single data array in the main file.
/// @param data the array of arrays
template <typename T>
void write_numpy_file(const std::string& filename, std::vector<std::vector<T>> data) {
    bool fortran_order = false;
    bool write_hdr_file = true;
    npy::npy_data<T> d;

    // flatten the vector of vectors, and save length of individual vectors in first entries.
    std::vector<int32_t> header = std::vector<int32_t>();
    for(int i = 0; i < data.size(); i++) {
        header.push_back(data[i].size());
    }

    std::vector<T> flat = flatten(data);
    auto merged = flat;
    //merged.insert(end(merged), begin(flat), end(flat));


    d.data = merged;
    d.shape = { merged.size() };
    d.fortran_order = fortran_order;
    npy::write_npy(filename, d);

    if(write_hdr_file) {
        const std::string& hdr_filename = filename + ".hdr";
        npy::npy_data<int32_t> h;
        h.data = header;
        h.shape = { header.size() };
        h.fortran_order = fortran_order;
        npy::write_npy(hdr_filename, h);
    }
}
