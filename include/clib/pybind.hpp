#pragma once

#include "image.hpp"
#include "Flexfloat.hpp"

namespace py = pybind11;
#include "pybind11/numpy.h"

namespace clib
{

img<Flexfloat> numpy_img_ff_set(const py::array &base)
{
    assert(base.ndim() == 2);
    assert(base.shape(0) > 0);
    assert(base.shape(1) > 0);
    assert(base.dtype().num() == 7); // int

    auto rows = base.shape(0);
    auto cols = base.shape(1);
    
    auto get_val = [&base](idx_t i, idx_t j) {
        float val = static_cast<float>(*(reinterpret_cast<const int *>(base.data(i, j))));
        return Flexfloat::from_float(8, 23, 127, val);
    };

    img<Flexfloat> res{};
    res._ctor_implt(rows, cols, get_val);

    return res;
}

py::array numpy_img_ff_get(const img<Flexfloat> &base)
{
    py::array_t<int> res;
    res.resize({base.rows(), base.cols()});
    
    auto set_val = [&base, &res](idx_t i, idx_t j) {
        *(res.mutable_data(i, j)) = static_cast<int>(base(i, j).to_float());
    };

    img<Flexfloat>::for_each(base.rows(), base.cols(), set_val);
    return res;
}



} // namespace clib