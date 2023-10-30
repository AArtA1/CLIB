#pragma once
#include "common.hpp"

namespace clib {

/*!
\brief
    Число с плавующей запятой с настриваемой мантиссой и экспонентой.

\details
    Число поддерживает денормализованные числа; нет NaN; нет ±inf
*/

using Stype = uint8_t;
using Etype = uint8_t;
using Mtype = uint8_t;
using Btype = int;

using etype = uint64_t;
using mtype = uint64_t;
using stype = uint8_t;


class Flexfloat
{
    Btype B;             // BIAS
    Etype E;             // EXPONENT WIDTH
    Mtype M;             // MANTISSA WIDTH
    const Stype S = 1;   // SIGN WIDTH

    stype s; // [0, 1]
    etype e; // [0, 2^E - 1]
    mtype m; // [0, 2^M - 1]

public:
    Flexfloat(Etype E_n, Mtype M_n, Btype B_n, stype s_n, etype e_n, mtype m_n);

    inline etype get_e() const noexcept { return e; }
    inline stype get_s() const noexcept { return s; }
    inline mtype get_m() const noexcept { return m; }
    inline Btype get_B() const noexcept { return B; }
    inline Mtype get_M() const noexcept { return M; }
    inline Etype get_E() const noexcept { return E; }

    inline etype max_exp() const;
    inline mtype max_mant() const;
    inline static etype max_exp(Etype E);
    inline static mtype max_mant(Mtype M);

    static Mtype msb(uint128_t val);
    Flexfloat& operator=(const Flexfloat& other);


    static void mult(const Flexfloat &left, const Flexfloat &right, Flexfloat &res);

    friend std::ostream &operator<<(std::ostream &oss, const Flexfloat &num);

private:

    // Fitting values in correct range
    //
    // Input: -inf <  cur_exp  < +inf
    //           0 <= cur_mant < +inf
    //
    // Desired output: Flexfloat with
    //           0 <= cur_exp  < 2^E
    //           0 <= cur_mant < 2^M
    //
    static Flexfloat normalise(uint8_t cur_sign, int128_t cur_exp, uint128_t cur_mant, Etype E, Mtype M, Btype B);

    bool is_valid() const;
};
}