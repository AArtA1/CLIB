#include "clib/Flexfixed.hpp"

#include "clib/logs.hpp"

namespace clib
{

#ifdef BOOST_LOGS
#define $(...) __VA_ARGS__
#else
#define $(...) ;
#endif

// #define LSB

// #define DEPRECATED_OPERATORS

using Itype = Flexfixed::Itype;
using Ftype = Flexfixed::Ftype;
using stype = Flexfixed::stype;
using ntype = Flexfixed::ntype;
using nrestype = Flexfixed::nrestype;
using wtype = Flexfixed::wtype;

Flexfixed::Flexfixed(Itype I_n, Ftype F_n) : I(I_n), F(F_n), s(0), n(0)
{
}

Flexfixed::Flexfixed(Itype I_n, Ftype F_n, nrestype val) : I(I_n), F(F_n), s(0), n(0)
{
    s = static_cast<stype>(val >> (I_n + F_n));
    n = static_cast<ntype>(((1u << (I_n + F_n)) - 1) & val);

    if (!is_valid())
    {
        $(CLOG(error) << "Can not create object. Invalid parameters");
        throw std::string{"Can not create object. Invalid parameters"};
    }
}

Flexfixed::Flexfixed(Itype I_n, Ftype F_n, stype s_n, ntype n_n) : I(I_n), F(F_n), s(s_n), n(n_n)
{
    if (!is_valid())
    {
        $(CLOG(error) << "Can not create object. Invalid parameters");
        throw std::string{"Can not create object. Invalid parameters"};
    }
}

// consider we already have res parameters: I and F
void Flexfixed::mult(const Flexfixed &lhs, const Flexfixed &rhs, Flexfixed &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "Addition of two numbers";
    Flexfixed::check_fxs({lhs, rhs, res});
    CLOG(trace) << "lhs  operand: " << lhs;
    CLOG(trace) << "rhs operand: " << rhs;
#endif

    res.s = lhs.s ^ rhs.s;

    wtype delta_f = static_cast<wtype>(lhs.F + rhs.F - res.F);

    $(CLOG(trace) << "DELTA_F:" << delta_f);

    nrestype res_n = lhs.n * rhs.n;

    if (delta_f >= 0)
    {
        res_n = res_n >> delta_f;
    }
    else
    {
        res_n = res_n << std::abs(delta_f);
    }

    res_n = check_ovf(res_n, res.I, res.F);

    assert(res_n <= std::numeric_limits<ntype>::max());

    res.n = static_cast<ntype>(res_n);

    $(CLOG(trace) << "Result of flex mult: " << res << std::endl);
}

void Flexfixed::sum(const Flexfixed &lhs, const Flexfixed &rhs, Flexfixed &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "Addition of two numbers";
    Flexfixed::check_fxs({lhs, rhs, res});
    CLOG(trace) << "lhs  operand: " << lhs;
    CLOG(trace) << "rhs operand: " << rhs;
#endif

    Ftype max_F = std::max(lhs.F, rhs.F);

    ntype lhs_n = lhs.n << (max_F - lhs.F), rhs_n = rhs.n << (max_F - rhs.F);

    bool flag = false;

    // check for |a| >= |b|
    // |a| < |b|
    if (!((lhs_n << max_F >= rhs_n << max_F) ||
          (lhs_n << max_F == rhs_n << max_F &&
           (((static_cast<ntype>(1) << max_F) - 1) & lhs_n) >= (((static_cast<ntype>(1) << max_F) - 1) & rhs_n))))
    {
        std::swap(lhs_n, rhs_n);
        flag = true;
    }

    nrestype res_n = lhs.s == rhs.s ? lhs_n + rhs_n : lhs_n - rhs_n;

    wtype delta_F = static_cast<wtype>(max_F - res.F);

    $(CLOG(trace) << "DELTA_F: " << delta_F);

    if (delta_F >= 0)
    {
        res_n = res_n >> delta_F;
    }
    else
    {
        res_n = res_n << delta_F;
    }

    res.s = flag ? rhs.s : lhs.s;

    res_n = check_ovf(res_n, res.I, res.F);

    assert(res_n <= std::numeric_limits<ntype>::max());

    res.n = static_cast<ntype>(res_n);

    $(CLOG(trace) << "Result of flex add: " << res << std::endl);
}

void Flexfixed::sub(const Flexfixed &lhs, const Flexfixed &rhs, Flexfixed &res)
{
    Flexfixed rhs_temp = rhs;

    if (rhs_temp.s == 0)
        rhs_temp.s = 1;
    else if (rhs_temp.s == 1)
        rhs_temp.s = 0;

    Flexfixed::sum(lhs, rhs_temp, res);
}

void Flexfixed::inv(const Flexfixed &val, Flexfixed &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "Number inv";
    Flexfixed::check_fxs({val, res});
    CLOG(trace) << "val: " << val;
#endif

    res.s = val.s;

    // overflow
    if (val.n == 0)
    {
        res.n = (1u << (res.I + res.F)) - 1u;
#ifdef BOOST_LOGS
        CLOG(trace) << "IS_OVERFLOW: TRUE";
        CLOG(trace) << "Result of val inv: " << res;
#endif
        return;
    }

    wtype L = msb(val);

    wtype R = static_cast<wtype>(L + static_cast<wtype>(1));

    nrestype res_n = ((1u << L) + (1u << R) - val.n) << (val.F + res.F);

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

    $(CLOG(trace) << "Result of val inv: " << res << std::endl);
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
        $(CLOG(trace) << "IS_OVERFLOW: TRUE");
        n = (static_cast<ntype>(1) << (I + F)) - 1;
    }
    else
    {
        $(CLOG(trace) << "IS_OVERFLOW: FALSE");
    }
    return n;
}


void Flexfixed::min(const Flexfixed &lhs, const Flexfixed &rhs, Flexfixed &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "min";
    Flexfixed::check_fxs({lhs, rhs, res});
    CLOG(trace) << "first: " << lhs;
    CLOG(trace) << "second: " << rhs;
#endif
    if (lhs > rhs)
        res = rhs;
    else
        res = lhs;
}

void Flexfixed::max(const Flexfixed &lhs, const Flexfixed &rhs, Flexfixed &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "max";
    Flexfixed::check_fxs({lhs, rhs, res});
    CLOG(trace) << "first: " << lhs;
    CLOG(trace) << "second: " << rhs;
#endif
    if (lhs < rhs)
        res = rhs;
    else
        res = lhs;
}

void Flexfixed::clip(const Flexfixed &a, const Flexfixed &x, const Flexfixed &b, Flexfixed &out)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "clip";
    Flexfixed::check_fxs({a, x, b, out});
    CLOG(trace) << "a: " << a;
    CLOG(trace) << "x: " << x;
    CLOG(trace) << "b: " << b;
#endif

    min(x, b, out);
    max(a, out, out);
}

Flexfixed Flexfixed::from_arithmetic_t(Flexfixed::Itype I_n, Flexfixed::Ftype F_n, float flt)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "Flexfixed from float = " << flt;
#endif

    Flexfixed result(I_n, F_n);
    result.s = flt > 0 ? 0 : 1;

    nrestype res_n = static_cast<nrestype>(fabs(flt) * (static_cast<ntype>(1) << F_n));

    res_n = check_ovf(res_n, I_n, F_n);

    assert(res_n <= std::numeric_limits<ntype>::max());
    result.n = static_cast<ntype>(res_n);

#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "Result = " << result;
#endif

    return result;
}
Flexfixed Flexfixed::from_arithmetic_t(const Flexfixed &hyperparams, float flt)
{
    return from_arithmetic_t(hyperparams.I, hyperparams.F, flt);
}

void Flexfixed::from_arithmetic_t(float flt, const Flexfixed &in, Flexfixed &out)
{
    out = from_arithmetic_t(in, flt);
}

Flexfixed Flexfixed::from_arithmetic_t(Itype I_n, Ftype F_n, long unsigned n)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "Flexfixed from long unsigned = " << n;
#endif
    if (n > (1 << I_n) - 1u)
    {
        throw std::runtime_error{"Flexfixed overflow. Can not fit long unsigned n in Flexfixed"};
    }

    Flexfixed result(I_n, F_n);
    result.s = 0;
    result.n = n << F_n;

    return result;
}
Flexfixed Flexfixed::from_arithmetic_t(const Flexfixed &hyperparams, long unsigned n)
{
    return from_arithmetic_t(hyperparams.I, hyperparams.F, n);
}
void Flexfixed::from_arithmetic_t(long unsigned n, const Flexfixed &in, Flexfixed &out)
{
    out = from_arithmetic_t(in, n);
}

Flexfixed Flexfixed::from_arithmetic_t(Itype I_n, Ftype F_n, int n)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "Flexfixed from int = " << n;
#endif
    stype sign = n > 0 ? 0 : 1;
    n = (n < 0) ? -n : n;
    unsigned n_uns = static_cast<unsigned>(n);

    if (n_uns > (1 << I_n) - 1u)
    {
        throw std::runtime_error{"Flexfixed overflow. Can not fit int n in Flexfixed"};
    }

    Flexfixed result(I_n, F_n);
    result.s = sign;
    result.n = n_uns << F_n;

    return result;
}
Flexfixed Flexfixed::from_arithmetic_t(const Flexfixed &hyperparams, int n)
{
    return from_arithmetic_t(hyperparams.I, hyperparams.F, n);
}
void Flexfixed::from_arithmetic_t(int n, const Flexfixed &in, Flexfixed &out)
{
    out = from_arithmetic_t(in, n);
}

float Flexfixed::to_float() const
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "Flexfixed to_float";
    CLOG(trace) << *this;
#endif

    float res = static_cast<float>(n) / static_cast<float>((static_cast<ntype>(1) << F));

    if (s == 1)
    {
        res = -res;
    }

    $(CLOG(trace) << res);

    return res;
}



bool operator>(const Flexfixed &lhs, const Flexfixed &rhs)
{
    assert(lhs.I == rhs.I);
    assert(lhs.F == rhs.F);

#ifndef DEPRECATED_OPERATORS
    // the same sign
    if (lhs.s == rhs.s)
    {
        return lhs.s == 0 ? lhs.n > rhs.n : lhs.n < rhs.n;
    }
    // sign is zero

    return lhs.s == 0 ? true : false;
#endif

#ifdef DEPRECATED_OPERATORS
    return lhs.to_float() > rhs.to_float();
#endif
}

bool operator>=(const Flexfixed &lhs, const Flexfixed &rhs)
{
    assert(lhs.I == rhs.I);
    assert(lhs.F == rhs.F);

    return lhs > rhs || lhs == rhs;
}

// todo: write without to_float
bool operator<(const Flexfixed &lhs, const Flexfixed &rhs)
{
    assert(lhs.I == rhs.I);
    assert(lhs.F == rhs.F);

#ifndef DEPRECATED_OPERATORS
    // the same sign
    if (lhs.s == rhs.s)
        // sign is zero
        return lhs.s == 0 ? lhs.n < rhs.n : lhs.n > rhs.n;

    return lhs.s == 0 ? true : false;
#endif

#ifdef DEPRECATED_OPERATORS
    return lhs.to_float() > rhs.to_float();
#endif
}

bool operator<=(const Flexfixed &lhs, const Flexfixed &rhs)
{
    return lhs < rhs || lhs == rhs;
}

bool operator==(const Flexfixed &lhs, const Flexfixed &rhs)
{
    return lhs.n == rhs.n && lhs.s == rhs.s;
}

bool operator!=(const Flexfixed &lhs, const Flexfixed &rhs)
{
    return !(lhs == rhs);
}

void Flexfixed::log2(const Flexfixed &val, Flexfixed &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "Number inv";
    Flexfixed::check_fxs({val, res});
    CLOG(trace) << "val: " << val;
#endif

    // overflow
    if (val.n == 0)
    {
        res.n = (static_cast<ntype>(1) << (res.I + res.F)) - 1;
#ifdef BOOST_LOGS
        CLOG(trace) << "IS_OVERFLOW: TRUE";
        CLOG(trace) << "Result of val inv: " << res;
#endif
        return;
    }

    wtype L = msb(val);

    nrestype res_n = (val.get_n() - (static_cast<nrestype>(1) << L)) << res.F;

#ifdef LSB
    uint8_t lsb = (res_n >> (L - 1)) % 2;
#endif

    res_n = res_n >> L;

#ifdef LSB
    res_n += lsb;
#endif

    if (L >= val.F)
    {
        res.s = 0;
        res_n = res_n + (static_cast<nrestype>(L - val.F) << res.F);
    }
    else
    {
        res.s = 1;
        res_n = (static_cast<nrestype>(val.F - L) << res.F) - res_n;
    }

    res_n = check_ovf(res_n, res.I, res.F);

    assert(res_n <= std::numeric_limits<ntype>::max());
    res.n = static_cast<ntype>(res_n);

    $(CLOG(trace) << "Result of val inv: " << res << std::endl);
}

void Flexfixed::abs(const Flexfixed &val, Flexfixed &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "abs";
    Flexfixed::check_fxs({val, res});
    CLOG(trace) << "Value: " << val;
    CLOG(trace) << "Result: " << res;
#endif

    assert(val.I == res.I);
    assert(val.F == res.F);

    res.n = val.n;
    res.s = 0;
    $(CLOG(trace) << "res: " << res);
}

void Flexfixed::negative(const Flexfixed &val, Flexfixed &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "negative";
    Flexfixed::check_fxs({val, res});
    CLOG(trace) << "Value: " << val;
    CLOG(trace) << "Result: " << res;
#endif

    assert(val.I == res.I);
    assert(val.F == res.F);

    res.n = val.n;
    res.s = val.s >= 1 ? 0 : 1;
    $(CLOG(trace) << "res: " << res);
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
            $(CLOG(error) << "Operand is invalid. Please check parameter correctness");
            throw std::runtime_error{"Invalid operand"};
        }
    }
}

bool Flexfixed::is_valid() const
{
    if (!(I + F <= sizeof(n) * 8))
    {
        $(CLOG(error) << "!(W=(I+F) <= sizeof(n)*8)");
        return false;
    }

    if (!(s <= 1))
    {
        $(CLOG(error) << "!(s <= 1)");
        return false;
    }
    if (!(n <= get_max_n(I, F)))
    {
        $(CLOG(error) << "!(n <= static_cast<ntype>((1 << (I+F)) - 1))");
        return false;
    }
    return true;
}

#ifdef LSB
#undef LSB
#endif

#ifdef DEPRECATED_OPERATORS
#undef DEPRECATED_OPERATORS
#endif

} // namespace clib