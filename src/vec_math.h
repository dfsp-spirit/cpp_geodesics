#pragma once

#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <cassert>
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

