#pragma once

#include "Flexfloat.hpp"
#include "Flexfixed.hpp"
#include "common.hpp"
#include "image.hpp"
#include "logs.hpp"

namespace clib
{
#pragma GCC diagnostic ignored "-Wattributes"

struct Synth
{
    [[synthesizer_func(Flexfloat::Const)]] //
    [[synthesizer_in(value, in)]]          //
    [[synthesizer_out(out)]]
    static void Flexfloat_Const(float value, const img<Flexfloat> &in, img<Flexfloat> &out);

    [[synthesizer_func(Flexfloat::sum)]] //
    [[synthesizer_in(in)]]               //
    [[synthesizer_out(out)]]
    static void Flexfloat_sum(const img<Flexfloat> &in, img<Flexfloat> &out);

    [[synthesizer_func(Flexfloat::mean)]] //
    [[synthesizer_in(in)]]               //
    [[synthesizer_out(out)]]
    static void Flexfloat_mean(const img<Flexfloat> &in, img<Flexfloat> &out);

    [[synthesizer_func(Flexfloat::Add)]] //
    [[synthesizer_in(lhs, rhs)]]         //
    [[synthesizer_out(res)]]
    static void Flexfloat_Add(const img<Flexfloat> &lhs, const img<Flexfloat> &rhs, img<Flexfloat> &res);


    [[synthesizer_func(Flexfloat::Mult)]] //
    [[synthesizer_in(lhs, rhs)]]          //
    [[synthesizer_out(res)]]
    static void Flexfloat_Mult(const img<Flexfloat> &lhs, const img<Flexfloat> &rhs, img<Flexfloat> &res);


    [[synthesizer_func(Flexfloat::Sub)]] //
    [[synthesizer_in(lhs, rhs)]]         //
    [[synthesizer_out(res)]]
    static void Flexfloat_Sub(const img<Flexfloat> &lhs, const img<Flexfloat> &rhs, img<Flexfloat> &res);

    [[synthesizer_func(Flexfloat::Inv)]] //
    [[synthesizer_in(x)]]                //
    [[synthesizer_out(res)]]
    static void Flexfloat_Inv(const img<Flexfloat> &x, img<Flexfloat> &res);

    [[synthesizer_func(Flexfixed::Const)]] //
    [[synthesizer_in(value, in)]]          //
    [[synthesizer_out(out)]]
    static void Flexfixed_Const(float value, const img<Flexfixed> &in, img<Flexfixed> &out);

    [[synthesizer_func(Flexfixed::sum)]] //
    [[synthesizer_in(in)]]               //
    [[synthesizer_out(out)]]
    static void Flexfixed_sum(const img<Flexfixed> &in, img<Flexfixed> &out);

    [[synthesizer_func(Flexfixed::mean)]] //
    [[synthesizer_in(in)]]               //
    [[synthesizer_out(out)]]
    static void Flexfixed_mean(const img<Flexfixed> &in, img<Flexfixed> &out);

    [[synthesizer_func(Flexfixed::Add)]] //
    [[synthesizer_in(lhs, rhs)]]         //
    [[synthesizer_out(res)]]
    static void Flexfixed_Add(const img<Flexfixed> &lhs, const img<Flexfixed> &rhs, img<Flexfixed> &res);

    [[synthesizer_func(Flexfixed::Mult)]] //
    [[synthesizer_in(lhs, rhs)]]          //
    [[synthesizer_out(res)]]
    static void Flexfixed_Mult(const img<Flexfixed> &lhs, const img<Flexfixed> &rhs, img<Flexfixed> &res);

    [[synthesizer_func(Flexfixed::Sub)]] //
    [[synthesizer_in(lhs, rhs)]]         //
    [[synthesizer_out(res)]]
    static void Flexfixed_Sub(const img<Flexfixed> &lhs, const img<Flexfixed> &rhs, img<Flexfixed> &res);

    [[synthesizer_func(Flexfixed::Inv)]] //
    [[synthesizer_in(x)]]                //
    [[synthesizer_out(res)]]
    static void Flexfixed_Inv(const img<Flexfixed> &x, img<Flexfixed> &res);

};
#pragma GCC diagnostic warning "-Wattributes"
} // namespace clib
