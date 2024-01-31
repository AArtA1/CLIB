#pragma once

#include "image.hpp"
#include "Flexfloat.hpp"
#include "polyfit.hpp"

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
        auto val = *(reinterpret_cast<const int *>(base.data(i, j)));
        return Flexfloat::from_arithmetic_t(8, 23, 127, val);
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
        *(res.mutable_data(i, j)) = base(i, j).to_int();
    };

    img<Flexfloat>::for_each(base.rows(), base.cols(), set_val);
    return res;
}

Flexfloat ff_set(float base, Flexfloat::Mtype E, Flexfloat::Mtype M, Flexfloat::Mtype B)
{
    return Flexfloat::from_arithmetic_t(E, M, B, base);
}

float ff_get(const Flexfloat &base)
{
    return base.to_float();
}
// float fractional_part(const Flexfloat &base, uint8_t F = 16)
// {
//     return static_cast<double>(base.fractional_part(F)) / (1 << F);
// }


polyfit_t polyfit_calc(const std::string &fname, polyfit_t l, unsigned L, unsigned K, bool L_base = true)
{
    return polyfit::get()->calc(fname, l, L, K, L_base);
}


} // namespace clib