#pragma once
#include "common.hpp"

namespace clib {

/*!
    \brief
        Число с фиксированной запятой.
    */

class FixedPoint
{
private:
    uint8_t I; // INT_WIDTH
    uint8_t F; // FRAC_WIDTH

    uint8_t s;  // sign
    uint64_t i; // integer part
    uint64_t f; // fractional part
public:
    FixedPoint() = default;
    FixedPoint(uint8_t I_n, uint8_t F_n, uint8_t s_n, uint64_t i_n, uint64_t f_n);

    static bool multiplication(const FixedPoint &left, const FixedPoint &right, FixedPoint &res);

    // get (integer.fractional) representation f.e (1000.001)
    uint128_t get_n() const;

    friend std::ostream &operator<<(std::ostream &oss, const FixedPoint &num);

private:
    bool is_valid() const;
};
}