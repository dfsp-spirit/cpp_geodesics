
#pragma once

#include "tinycolormap.hpp"

#include <vector>
#include <algorithm>

// Functions for mapping numerical values to colors using a colormap function.


/// Normalize values to range 0..1.
/// @param data vector of at least two unique and finite numbers
/// @param force whether to return zeros instead of throwing exceptions in corner cases.
/// @throws std::invalid_argument if values are empty or max is equal to min.
template<class T>
std::vector<T> normalize(const std::vector<T> data) {
    if(data.size() < 2) {
        throw std::invalid_argument("The 'data' vector to normalize must contain at least 2 elements, but size is " + std::to_string(data.size()) + ".");
    }
    T min = *std::min_element(data.begin(), data.end());
    T max = *std::max_element(data.begin(), data.end());
    if(min == max) {
        throw std::invalid_argument("The 'data' vector to normalize must contain at least 2 unique elements, but all " + std::to_string(data.size()) + " elements are equal.");
    }
    T range = max - min;

    std::vector<T> scaled(data.size());
    for(size_t i=0; i<data.size(); i++) {
        scaled[i] = (data[i] - min) / range;
    }
    return(scaled);
}


/// Map n data values to a vector of 3n unit_8 values, which represent the RGB channels of the respective colors.
/// @details In the returned vector, three consecutive values describe the RGB data for one value. This must currently be called with double data, as the colormap function only works with double. See specializations below, which enable usage with float and int.
/// @throws std::invalid_argument if values are empty or max is equal to min.
template<class T>
std::vector<uint8_t> data_to_colors(const std::vector<T> data, const tinycolormap::ColormapType cmap = tinycolormap::ColormapType::Viridis) {
    std::vector<T> dnorm = normalize(data);
    std::vector<uint8_t> colors;
    colors.reserve(data.size() * 3);
    for(size_t i=0; i<dnorm.size(); i++) {
        tinycolormap::Color color = tinycolormap::GetColor(dnorm[i], cmap);
        colors.push_back(color.ri());
        colors.push_back(color.gi());
        colors.push_back(color.bi());
    }
    return(colors);
}

/// Template specialization for float, so users do not need to do the conversion themselves on each usage.
template<>
std::vector<uint8_t> data_to_colors(const std::vector<float> data, const tinycolormap::ColormapType cmap) {
    return(data_to_colors(std::vector<double>(data.begin(), data.end()), cmap));
}

/// Template specialization for int, so users do not need to do the conversion themselves on each usage.
template<>
std::vector<uint8_t> data_to_colors(const std::vector<int> data, const tinycolormap::ColormapType cmap) {
    return(data_to_colors(std::vector<double>(data.begin(), data.end()), cmap));
}








