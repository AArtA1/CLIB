#include <clib/synth.hpp>


namespace clib
{
void Synth::Flexfloat_Const(float value, const img<Flexfloat> &in, img<Flexfloat> &out)
{
    Flexfloat prototype;
    Flexfloat::from_arithmetic_t(value, in.vv()[0][0], prototype);

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

void Synth::Flexfixed_Const(float value, const img<Flexfixed> &in, img<Flexfixed> &out)
{
    Flexfixed prototype;
    Flexfixed::from_arithmetic_t(value, in.vv()[0][0], prototype);

    out = std::move(img<Flexfixed>(prototype, in.rows(), in.cols()));
}

void Synth::Flexfixed_sum(const img<Flexfixed> &in, img<Flexfixed> &out)
{
    Flexfixed fout = in.sum();
    out = img<Flexfixed>{fout, in.rows(), in.cols()};
}

void Synth::Flexfixed_mean(const img<Flexfixed> &in, img<Flexfixed> &out)
{
    Flexfixed fout = in.mean();
    out = img<Flexfixed>{fout, in.rows(), in.cols()};
}

void Synth::Flexfixed_Add(const img<Flexfixed> &lhs, const img<Flexfixed> &rhs, img<Flexfixed> &res)
{
    img<Flexfixed>::add(lhs, rhs, res);
}

void Synth::Flexfixed_Mult(const img<Flexfixed> &lhs, const img<Flexfixed> &rhs, img<Flexfixed> &res)
{
    img<Flexfixed>::mult(lhs, rhs, res);
}

void Synth::Flexfixed_Sub(const img<Flexfixed> &lhs, const img<Flexfixed> &rhs, img<Flexfixed> &res)
{
    img<Flexfixed>::sub(lhs, rhs, res);
}

void Synth::Flexfixed_Inv(const img<Flexfixed> &x, img<Flexfixed> &res)
{
    img<Flexfixed>::inv(x, res);
}

} // namespace clib