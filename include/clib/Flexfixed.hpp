#pragma once
#include "common.hpp"

namespace clib {

using Itype = uint8_t; // size of INT_WIDTH 
using Ftype = uint8_t; // size of FRAC_WIDTH

using stype = uint8_t; // size of SIGN
using ntype = uint64_t; // size of NUMERATOR


class FlexFixed
{
private:
    Itype I; // INT_WIDTH
    Ftype F; // FRAC_WIDTH

    stype s;  // s
    ntype n; // numerator
public:
    FlexFixed() = default;

    FlexFixed(Itype I_n,Ftype F_n);

    FlexFixed(Itype I_n, Ftype F_n, stype s_n,ntype n_n);

    FlexFixed& operator=(const FlexFixed& other);

    static void multiplication(const FlexFixed& left, const FlexFixed & right, FlexFixed & res);

    static void addition(const FlexFixed& left, const FlexFixed& right, FlexFixed& res);

    static void substraction(const FlexFixed& left,const FlexFixed& right, FlexFixed& res);

    friend bool operator>(const FlexFixed& left,const FlexFixed& right);

    friend std::ostream &operator<<(std::ostream &oss, const FlexFixed &num);

    bool is_valid() const;
    
    ntype get_int() const;

    ntype get_frac() const;

    ntype get_n() const;

};

}