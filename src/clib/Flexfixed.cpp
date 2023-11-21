#include "clib/Flexfixed.hpp"

#include "clib/logs.hpp"

namespace clib
{

//#define LSB

Flexfixed::Flexfixed(Itype I_n, Ftype F_n) : I(I_n), F(F_n), s(0), n(0)
{
}

Flexfixed::Flexfixed(Itype I_n, Ftype F_n, nrestype value) : I(I_n), F(F_n), s(0), n(0)
{
    s = static_cast<stype>(value >> (I_n + F_n));
    n = static_cast<ntype>(((static_cast<ntype>(1) << (I_n + F_n)) - 1) & value);

    if (!is_valid())
    {
        CLOG(error) << "Can not create object. Invalid parameters";
        throw std::string{"Can not create object. Invalid parameters"};
    }
}

Flexfixed::Flexfixed(Itype I_n, Ftype F_n, stype s_n, ntype n_n) : I(I_n), F(F_n), s(s_n), n(n_n)
{
    if (!is_valid())
    {
        CLOG(error) << "Can not create object. Invalid parameters";
        throw std::string{"Can not create object. Invalid parameters"};
    }
}

// consider we already have res parameters: I and F
void Flexfixed::mult(const Flexfixed &left, const Flexfixed &right, Flexfixed &res)
{
#ifndef NDEBUG
    CLOG(trace) << "Addition of two numbers";
    check_fxs({left, right, res});
    CLOG(trace) << "Left  operand: " << left;
    CLOG(trace) << "Right operand: " << right;
#endif

    res.s = left.s ^ right.s;

    wtype delta_f = static_cast<wtype>(left.F + right.F - res.F);

    CLOG(trace) << "DELTA_F:" << delta_f;

    nrestype res_n = left.n * right.n;

    if (delta_f >= 0)
    {
        res_n = res_n >> delta_f;
    }
    else
    {
        res_n = res_n << abs(delta_f);
    }

    // overflow
    res_n = check_ovf(res_n, res.I, res.F);

    assert(res_n <= std::numeric_limits<ntype>::max());

    res.n = static_cast<ntype>(res_n);

#ifndef NDEBUG
    CLOG(trace) << "Result of flex mult: " << res << std::endl;
#endif
}

void Flexfixed::add(const Flexfixed &left, const Flexfixed &right, Flexfixed &res)
{
#ifndef NDEBUG
    CLOG(trace) << "Addition of two numbers";
    check_fxs({left, right, res});
    CLOG(trace) << "Left  operand: " << left;
    CLOG(trace) << "Right operand: " << right;
#endif

    Ftype max_F = std::max(left.F, right.F);

    ntype left_n = left.n << (max_F - left.F), right_n = right.n << (max_F - right.F);

    bool flag = false;

    // check for |a| >= |b|
    // |a| < |b|
    if (!((left_n << max_F >= right_n << max_F) ||
          (left_n << max_F == right_n << max_F &&
           (((static_cast<ntype>(1) << max_F) - 1) & left_n) >= (((static_cast<ntype>(1) << max_F) - 1) & right_n))))
    {
        std::swap(left_n, right_n);
        flag = true;
    }

    nrestype res_n = left.s == right.s ? left_n + right_n : left_n - right_n;

    wtype delta_F = static_cast<wtype>(max_F) - res.F;

    CLOG(trace) << "DELTA_F: " << delta_F;

    if (delta_F >= 0)
    {
        res_n = res_n >> delta_F;
    }
    else
    {
        res_n = res_n << delta_F;
    }

    res.s = flag ? right.s : left.s;

    // overflow
    res_n = check_ovf(res_n, res.I, res.F);

    // todo : add assert to prevent unexcepted errors

    assert(res_n <= std::numeric_limits<ntype>::max());

    res.n = static_cast<ntype>(res_n);

#ifndef NDEBUG
    CLOG(trace) << "Result of flex add: " << res << std::endl;
#endif
}

void Flexfixed::substraction(const Flexfixed &left, const Flexfixed &right, Flexfixed &res)
{
    Flexfixed right_temp = right;

    if (right_temp.s == 0)
    {
        right_temp.s = 1;
    }
    else
    {
        if (right_temp.s == 1)
        {
            right_temp.s = 0;
        }
    }

    Flexfixed::add(left, right_temp, res);
}

void Flexfixed::inv(const Flexfixed &value, Flexfixed &res)
{
#ifndef NDEBUG
    CLOG(trace) << "Number inv";
    check_fxs({value, res});
    CLOG(trace) << "Value: " << value;
#endif

    res.s = value.s;

    // overflow
    if (value.n == 0)
    {
        res.n = (static_cast<ntype>(1) << (res.I + res.F)) - 1;
#ifndef NDEBUG
        CLOG(trace) << "IS_OVERFLOW: TRUE";
        CLOG(trace) << "Result of value inv: " << res;
#endif
        return;
    }

    wtype L = msb(value);

    wtype R = L + 1;

    nrestype res_n = ((static_cast<ntype>(1) << L) + (static_cast<ntype>(1) << R) - value.n) << (value.F + res.F);

#ifdef LSB
    uint8_t lsb = (res_n >> (L + R - 1)) % 2;
#endif

    res_n = res_n >> (L + R);

#ifdef LSB
    res_n += lsb;
#endif

    // overflow
    res_n = check_ovf(res_n, res.I, res.F);

    assert(res_n <= std::numeric_limits<ntype>::max());
    res.n = static_cast<ntype>(res_n);

#ifndef NDEBUG
    CLOG(trace) << "Result of value inv: " << res << std::endl;
#endif
}

Flexfixed::wtype Flexfixed::msb(const Flexfixed &val)
{
    wtype r = 0;
    ntype n_t = val.n;
    while (n_t >>= 1)
    {
        ++r;
    }
    return r;
}

Flexfixed::nrestype Flexfixed::check_ovf(Flexfixed::nrestype n, Flexfixed::Itype I, Flexfixed::Ftype F)
{
    if (n >= static_cast<ntype>(1) << (I + F))
    {
#ifndef NDEBUG
        CLOG(trace) << "IS_OVERFLOW: TRUE";
#endif
        n = (static_cast<ntype>(1) << (I + F)) - 1;
    }
    else
    {
#ifndef NDEBUG
        CLOG(trace) << "IS_OVERFLOW: FALSE";
#endif
    }
    return n;
}


Flexfixed Flexfixed::from_float(Itype I, Ftype F, float flt)
{
#ifndef NDEBUG
    CLOG(trace) << std::endl;
    CLOG(trace) << "Flexfixed from_float = " << flt;
#endif

    Flexfixed result(I, F);
    result.s = flt > 0 ? 0 : 1;

    nrestype res_n = static_cast<nrestype>(fabs(flt) * (static_cast<ntype>(1) << F));

    res_n = check_ovf(res_n, I, F);

    assert(res_n <= std::numeric_limits<ntype>::max());
    result.n = static_cast<ntype>(res_n);

#ifndef NDEBUG
    CLOG(trace) << std::endl;
    CLOG(trace) << "Result = " << result;
#endif

    return result;
}

float Flexfixed::to_float() const
{
#ifndef NDEBUG
    CLOG(trace) << std::endl;
    CLOG(trace) << "Flexfixed to_float";
    CLOG(trace) << *this;
#endif

    float res = static_cast<float>(n) / (static_cast<ntype>(1) << F);

    if (s == 1)
    {
        res = -res;
    }

#ifndef NDEBUG
    CLOG(trace) << res;
#endif

    return res;
}

bool operator>(const Flexfixed &left, const Flexfixed &right)
{
    // todo for different signs
    return left.get_int() != right.get_int() ? left.get_int() > right.get_int() : left.get_frac() > right.get_frac();
}

std::string Flexfixed::bits() const
{
    std::stringstream ostream;

    ostream << std::bitset<1>(s) << "|" << to_string_int() << "|" << to_string_frac();

    return ostream.str();
}

std::string Flexfixed::bits(const Flexfixed &fx) const
{
    std::stringstream ostream;

    ostream << std::bitset<1>(s) << "|" << std::setw(fx.I) << to_string_int() << "|" << to_string_frac()
            << std::setw(fx.F);

    return ostream.str();
}

std::ostream &operator<<(std::ostream &oss, const Flexfixed &num)
{
    oss << "(S, I, F) = (" << 1 << ", " << +num.I << ", " << +num.F << ")";
    oss << "  val = " << num.bits();

    return oss;
}

void Flexfixed::check_fxs(std::initializer_list<Flexfixed> list)
{
    for (auto &elem : list)
    {
        if (!elem.is_valid())
        {
            CLOG(error) << "Operand is invalid. Please check parameter correctness";
            throw std::runtime_error{"Invalid operand"};
        }
    }
}

bool Flexfixed::is_valid() const
{
    if (!(I + F <= sizeof(n) * 8))
    {
        CLOG(error) << "!(W=(I+F) <= sizeof(n)*8)";
        return false;
    }

    if (!(s <= 1))
    {
        CLOG(error) << "!(s <= 1)";
        return false;
    }
    if (!(n <= get_max_n(I, F)))
    {
        CLOG(error) << "!(n <= static_cast<ntype>((1 << (I+F)) - 1))";
        return false;
    }
    return true;
}

#ifdef LSB
#undef LSB
#endif

} // namespace clib