#pragma once
#include "common.hpp"
#include "video.hpp"

namespace clib{


std::pair<int,int> transform_coordinates(const vector<vector<int>>& vv_, std::pair<int,int> coordinates,std::pair<int,int> center){
    if(coordinates.first < 0){
        coordinates.first = 2 * center.first - coordinates.first;
    }
    if(coordinates.first >= vv_.size()){
        coordinates.first =  2 * center.first - coordinates.first;
    }
    if(coordinates.second < 0){
        coordinates.second = 2 * center.second - coordinates.second;
    }
    if(coordinates.second >= vv_[0].size()){
        coordinates.second = 2 * center.second - coordinates.second ;
    }
    return coordinates;
}

template<typename T>
std::vector<std::vector<T>> get_window(const img<T> image, std::pair<size_t,size_t> center, std::pair<size_t,size_t> shape){
    assert(center.first < image.rows());
    assert(center.second < images.cols());

    assert(shape.first < image.rows() && shape.first % 2 == 0);
    assert(shape.second < image.cols() && shape.second % 2 == 0);

    auto rows = image.rows();
    auto cols = image.cols();

    int top = static_cast<int>(center.first) - shape.first / 2;
    int left = static_cast<int>(center.second) - shape.second / 2;

    vector<vector<int>> res_window(shape.first, vector<int>(shape.second, 0));

    // Copy elements from the original matrix to the new one
    for (int i = 0; i < shape.first; ++i) {
        for (int j = 0; j < shape.second; ++j) {
            auto res_coord = transform_coordinates(vv_,{top + i, left + j},{center.first,center.second});
            res_window[i][j] = vv_[res_coord.first][res_coord.second];
        }
    }

    return res_window;
}



}