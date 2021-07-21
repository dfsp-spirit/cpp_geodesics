#pragma once

#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <cassert>
#include <cmath>
#include <functional>


// Init vector of specified length with identical values.
template<typename T>
std::vector<T> vinit(T t, size_t times) {  
  std::vector<T> res(times);
  for(size_t i=0; i<times; i++) {
    res[i] = t;
  }
  return res;
}


template <typename T>
std::vector<T> operator+(const std::vector<T>& a, const std::vector<T>& b)
{
    assert(a.size() == b.size());

    std::vector<T> result;
    result.reserve(a.size());

    std::transform(a.begin(), a.end(), b.begin(), 
                   std::back_inserter(result), std::plus<T>());
    return result;
}

/// Compute the Frobenius norm of a vector, i.e., its Euclidean length.
template<typename Iter_T>
long double vnorm(Iter_T t) {
  return sqrt(inner_product(t.begin(), t.end(), t.begin(), 0.0L));
}

template <typename T>
std::vector<T> operator*(const std::vector<T>& a, const std::vector<T>& b)
{
    assert(a.size() == b.size());

    std::vector<T> result(a.size());
    for(size_t i=0; i<a.size(); i++) {
        result[i] = a[i] * b[i];
    }

    return result;
}

// Cross product of two 3-dimensional vectors.
template <typename T>
std::vector<T> cross(const std::vector<T>& a, const std::vector<T>& b)
{
    assert(a.size() == b.size());
    assert(a.size() == 3);

    std::vector<T> result(3);
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];

    return result;
}

template <typename T>
std::vector<T> operator*(T t, const std::vector<T>& v)
{
    std::vector<T> a = vinit(t, v.size());
    assert(a.size() == v.size());

    return a*v;
}

template <typename T>
std::vector<T> operator-(const std::vector<T>& a, const std::vector<T>& b)
{
    assert(a.size() == b.size());

    std::vector<T> result(a.size());
    for(size_t i=0; i<a.size(); i++) {
        result[i] = a[i] - b[i];
    }

    return result;
}

template <typename T>
std::vector<T> operator+(const T t, const std::vector<T>& v)
{

    std::vector<T> result;
    result.resize(v.size());

    for(size_t i=0; i<v.size(); i++) {
        result[i] = v[i] + t;
    }
    return result;
}

