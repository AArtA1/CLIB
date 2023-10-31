#include "clib/Fixedpoint.hpp"
#include "clib/logs.hpp"

namespace clib {

FixedPoint::FixedPoint(uint8_t I_n, uint8_t F_n, uint8_t s_n, uint64_t n_n) : 
    I(I_n), F(F_n), s(s_n), n(n_n)
{
    if (!is_valid())
    {
        CLOG(error) << "Can not create object. Invalid parameters";
        throw std::string{"Can not create object. Invalid parameters"};
    }

    CLOG(trace) << "Object successfully created";
}

bool FixedPoint::multiplication(const FixedPoint &left, const FixedPoint &right, FixedPoint &res)
{

    CLOG(trace) << "Multiplication of two numbers";

    if (!left.is_valid())
    {
        CLOG(error) << "Left operand is invalid";
        throw std::string{"Invalid operand"};
    }

    if (!left.is_valid())
    {
        CLOG(error) << "Right operand is invalid";
        throw std::string{"Can not create object. Invalid parameters"};
    }

    CLOG(trace) << "Left operand: " << left;
    CLOG(trace) << "Right operand: " << right;

    res.s = left.s ^ right.s;
    res.I = left.I;
    res.F = left.F;

    // n_res = (n_a * n_b) >> left.f
    uint128_t n_res = (left.n * right.n) >> left.F;

    // overflow n_res > 2^(I+F) - 1
    if (n_res > (static_cast<uint128_t>(1) << (left.I + left.F)) - 1)
    {
        // todo
        return false;
    }   


    res.n = static_cast<uint64_t>(n_res);

    // res.i = n_res >> res.F;
    // res.f = ((1 << res.F) - 1) & n_res;
    CLOG(trace) << "Result of multiplication: " << res;
    return true;
}


std::ostream& operator<<(std::ostream &oss, const FixedPoint &num)
{
    oss << "INT_WIDTH:" << static_cast<int>(num.I) << "\n";
    oss << "FRAC_WIDTH:" << static_cast<int>(num.F) << "\n";

    std::string sign_s = std::bitset<8>(num.s).to_string();
    std::string int_s = std::bitset<64>(num.get_int()).to_string();
    std::string frac_s = std::bitset<64>(num.get_frac()).to_string();

    sign_s = sign_s.substr(8 - 1, std::string::npos);
    int_s = int_s.substr(64 - num.I, std::string::npos);
    frac_s = frac_s.substr(64 - num.F, std::string::npos);

    oss << "Sign: " << sign_s << std::endl;
    oss << "Int:  " << int_s << std::endl;
    oss << "Frac: " << frac_s << std::endl;
    oss << "Presentation of fixed_point: " << int_s << "." << frac_s << std::endl;
    return oss;
}

bool FixedPoint::is_valid() const
{
    uint64_t i = get_int(),f = get_frac();
    if (!(s <= 1))
        goto ivalid_obj;
    if (!(i <= static_cast<uint64_t>((1 << I) - 1)))
        goto ivalid_obj;
    if (!(f <= static_cast<uint64_t>((1 << F) - 1)))
        goto ivalid_obj;

    CLOG(trace) << "Object is valid";
    return true;

ivalid_obj:
    CLOG(error) << "Object is invalid";
    return false;
}

uint64_t FixedPoint::get_int() const{
    return n >> F;
}

uint64_t FixedPoint::get_frac() const{
    return ((static_cast<uint64_t>(1) << F) - 1) & n;
}

uint64_t FixedPoint::get_n() const{
    return n;
}

}