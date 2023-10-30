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

    uint8_t s;  // s
    uint64_t n; // numerator
public:
    FixedPoint() = default;

    FixedPoint(uint8_t I_n, uint8_t F_n, uint8_t s_n,uint64_t n_n);

    static bool multiplication(const FixedPoint &left, const FixedPoint &right, FixedPoint &res);

    friend std::ostream &operator<<(std::ostream &oss, const FixedPoint &num);

    bool is_valid() const;
    
    uint64_t get_int() const;

    uint64_t get_frac() const;

    uint64_t get_n() const;

};


}