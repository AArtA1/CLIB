#include "clib/Fixedpoint.hpp"
#include "clib/logs.hpp"

namespace clib {

FixedPoint::FixedPoint(uint8_t I_n, uint8_t F_n, uint8_t s_n, uint64_t i_n, uint64_t f_n) : 
    I(I_n), F(F_n), s(s_n), i(i_n), f(f_n)
{
    if (!is_valid())
    {
        LOG(error) << "Can not create object. Invalid parameters";
        throw std::string{"Can not create object. Invalid parameters"};
    }

    LOG(trace) << "Object successfully created";
}

bool FixedPoint::multiplication(const FixedPoint &left, const FixedPoint &right, FixedPoint &res)
{

    LOG(trace) << "Multiplication of two numbers";

    if (!left.is_valid())
    {
        LOG(error) << "Left operand is invalid";
        throw std::string{"Invalid operand"};
    }

    if (!left.is_valid())
    {
        LOG(error) << "Right operand is invalid";
        throw std::string{"Can not create object. Invalid parameters"};
    }

    LOG(trace) << "Left operand: " << left;
    LOG(trace) << "Right operand: " << right;

    res.s = left.s ^ right.s;
    res.I = left.I;
    res.F = left.F;
    // n_res = (n_a * n_b) >> left.f
    uint128_t n_res = (left.get_n() * right.get_n()) >> left.f;

    // overflow n_res > 2^(I+F) - 1
    if (n_res > (static_cast<uint128_t>(1) << (left.I + left.F)) - 1)
    {
        // todo
        return false;
    }   

    // res.i = n_res >> res.F;
    // res.f = ((1 << res.F) - 1) & n_res;
    LOG(trace) << "Result of multiplication: " << res;
    return true;
}

// get (integer.fractional) representation f.e (1000.001)
uint128_t FixedPoint::get_n() const
{
    return ((static_cast<uint128_t>(i) << F) | f);
}

std::ostream& operator<<(std::ostream &oss, const FixedPoint &num)
{
    oss << "INT_WIDTH:" << static_cast<int>(num.I) << "\n";
    oss << "FRAC_WIDTH:" << static_cast<int>(num.F) << "\n";

    std::string sign_s = std::bitset<8>(num.s).to_string();
    std::string int_s = std::bitset<64>(num.i).to_string();
    std::string frac_s = std::bitset<64>(num.f).to_string();

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
    if (!(s <= 1))
        goto ivalid_obj;
    if (!(i <= static_cast<uint64_t>((1 << I) - 1)))
        goto ivalid_obj;
    if (!(f <= static_cast<uint64_t>((1 << f) - 1)))
        goto ivalid_obj;

    LOG(trace) << "Object is valid";
    return true;

ivalid_obj:
    LOG(error) << "Object is invalid";
    return false;
}

}