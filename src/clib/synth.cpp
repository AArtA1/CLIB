#include <clib/synth.hpp>


namespace clib
{
void Synth::Flexfloat_Const(float value, const img<Flexfloat> &in, img<Flexfloat> &out)
{
    Flexfloat prototype;
    Flexfloat::from_float(value, in.vv()[0][0], prototype);

    out = std::move(img<Flexfloat>(prototype, in.rows(), in.cols()));
}

void Synth::Flexfloat_sum(const img<Flexfloat> &in, img<Flexfloat> &out)
{
    Flexfloat fout = in.sum();
    out = img<Flexfloat>{fout, in.rows(), in.cols()};
}

void Synth::Flexfloat_mean(const img<Flexfloat> &in, img<Flexfloat> &out)
{
    Flexfloat fout = in.mean();
    out = img<Flexfloat>{fout, in.rows(), in.cols()};
}


void Synth::Flexfloat_Add(const img<Flexfloat> &lhs, const img<Flexfloat> &rhs, img<Flexfloat> &res)
{
    img<Flexfloat>::add(lhs, rhs, res);
}

void Synth::Flexfloat_Mult(const img<Flexfloat> &lhs, const img<Flexfloat> &rhs, img<Flexfloat> &res)
{
    img<Flexfloat>::mult(lhs, rhs, res);
}

void Synth::Flexfloat_Sub(const img<Flexfloat> &lhs, const img<Flexfloat> &rhs, img<Flexfloat> &res)
{
    img<Flexfloat>::sub(lhs, rhs, res);
}

void Synth::Flexfloat_Inv(const img<Flexfloat> &x, img<Flexfloat> &res)
{
    img<Flexfloat>::inv(x, res);
}

} // namespace clib