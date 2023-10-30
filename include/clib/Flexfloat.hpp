#pragma once
#include "common.hpp"

namespace clib {

/*!
\brief
    Число с плавующей запятой с настриваемой мантиссой и экспонентой.

\details
    Число поддерживает денормализованные числа; нет NaN; нет ±inf
*/
class Flexfloat
{
    int B;               // BIAS
    uint8_t E;           // EXPONENT WIDTH
    uint8_t M;           // MANTISSA WIDTH
    const uint8_t S = 1; // SIGN WIDTH

    uint8_t s;  // [0, 1]
    uint64_t e; // [0, 2^E - 1]
    uint64_t m; // [0, 2^M - 1]

public:
    Flexfloat(uint8_t E_n, uint8_t M_n, int B_n, uint8_t s_n, uint64_t e_n, uint64_t m_n);


    inline uint64_t get_e() const noexcept { return e; }
    inline uint8_t get_s()  const noexcept { return s; }
    inline uint64_t get_m() const noexcept { return m; }
    inline int get_B()      const noexcept { return B; }
    inline uint8_t get_M()  const noexcept { return M; }
    inline uint8_t get_E()  const noexcept { return E; }

    inline uint64_t max_exp() const;
    inline uint64_t max_mant() const;
    inline static uint64_t max_exp(uint8_t E);
    inline static uint64_t max_mant(uint8_t M);

    static uint8_t msb(uint128_t val);
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
    static Flexfloat normalise(uint8_t cur_sign, int128_t cur_exp, uint128_t cur_mant, uint8_t E, uint8_t M, int B);

    bool is_valid() const;
};
}