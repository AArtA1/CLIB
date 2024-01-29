#include "clib/logs.hpp"
#include "clib/polyfit.hpp"
#include <clib/Flexfloat.hpp>

#include <ieee754.h>

namespace clib
{

#ifdef BOOST_LOGS
#define $(...) __VA_ARGS__
#else
#define $(...) ;
#endif

Flexfloat::Flexfloat() : B(0), E(0), M(0), s(0), e(0), m(0)
{
}

Flexfloat::Flexfloat(Etype E_n, Mtype M_n, Btype B_n, stype s_n, etype e_n, mtype m_n)
    : B(B_n), E(E_n), M(M_n), s(s_n), e(e_n), m(m_n)
{
    if (!is_valid())
    {
#ifdef BOOST_LOGS
        CLOG(error) << "Can not create object. Invalid parameters";
        CLOG(error) << "E = " << E;
        CLOG(error) << "M = " << M;
        CLOG(error) << "B = " << B;
        CLOG(error) << "s = " << +s;
        CLOG(error) << "e = " << +e;
        CLOG(error) << "m = " << +m;
        CLOG(error) << "Can not create object. Invalid parameters";
#endif
        throw std::runtime_error{"Can not create object. Invalid parameters"};
    }
}

Flexfloat::Flexfloat(Etype E_n, Mtype M_n, Btype B_n, uint64_t value) : B(B_n), E(E_n), M(M_n), s(0), e(0), m(0)
{
    assert(msb(value) + 1 <= M + B + S);

    m = static_cast<mtype>((value >> 0) & ((1u << M) - 1u));
    e = static_cast<etype>((value >> M) & ((1u << E) - 1u));
    s = static_cast<stype>((value >> (M + E)) & ((1u << S) - 1u));

    if (!is_valid())
    {
#ifdef BOOST_LOGS
        CLOG(error) << "Can not create object. Invalid parameters";
        CLOG(error) << "E = " << E;
        CLOG(error) << "M = " << M;
        CLOG(error) << "B = " << B;
        CLOG(error) << "s = " << +s;
        CLOG(error) << "e = " << +e;
        CLOG(error) << "m = " << +m;
#endif
        throw std::runtime_error{"Can not create object. Invalid parameters"};
    }
}

Flexfloat::Flexfloat(const Flexfloat &hyperparams, uint64_t value) : B(0), E(0), M(0), s(0), e(0), m(0)
{
    B = hyperparams.get_B();
    E = hyperparams.get_E();
    M = hyperparams.get_M();

    assert(msb(value) + 1 <= M + B + S);
    m = static_cast<mtype>((value >> 0) & ((1u << M) - 1u));
    e = static_cast<etype>((value >> M) & ((1u << E) - 1u));
    s = static_cast<stype>((value >> (M + E)) & ((1u << S) - 1u));

    if (!is_valid())
    {
#ifdef BOOST_LOGS
        CLOG(error) << "Can not create object. Invalid parameters";
        CLOG(error) << "E = " << E;
        CLOG(error) << "M = " << M;
        CLOG(error) << "B = " << B;
        CLOG(error) << "s = " << +s;
        CLOG(error) << "e = " << +e;
        CLOG(error) << "m = " << +m;
#endif
        throw std::runtime_error{"Can not create object. Invalid parameters"};
    }
}

Flexfloat Flexfloat::pack(const Flexfloat &in, hyper_params req_hyperparams)
{
    mexttype in_m = unzip(in);
    return normalise(in.s, in.e, in_m, in.M, req_hyperparams);
}

Flexfloat Flexfloat::ovf(Etype E_n, Mtype M_n, Btype B_n, stype s_n)
{
    return Flexfloat(E_n, M_n, B_n, s_n, max_exp(E_n), max_mant(M_n));
}

Flexfloat Flexfloat::zero(Etype E_n, Mtype M_n, Btype B_n, stype s_n)
{
    return Flexfloat(E_n, M_n, B_n, s_n, 0, 0);
}

Flexfloat Flexfloat::min_denorm(Etype E_n, Mtype M_n, Btype B_n, stype s_n)
{
    return Flexfloat(E_n, M_n, B_n, s_n, 0, 1);
}

Flexfloat Flexfloat::max_denorm(Etype E_n, Mtype M_n, Btype B_n, stype s_n)
{
    return Flexfloat(E_n, M_n, B_n, s_n, 0, max_mant(M_n));
}

Flexfloat Flexfloat::min_norm(Etype E_n, Mtype M_n, Btype B_n, stype s_n)
{
    return Flexfloat(E_n, M_n, B_n, s_n, 1, 1);
}

Flexfloat Flexfloat::max_norm(Etype E_n, Mtype M_n, Btype B_n, stype s_n)
{
    return Flexfloat(E_n, M_n, B_n, s_n, max_exp(E_n), max_mant(M_n));
}
Flexfloat Flexfloat::max_norm() const
{
    return Flexfloat(E, M, B, s, max_exp(), max_mant());
}

Flexfloat::etype Flexfloat::max_exp() const
{
    return (1 << E) - 1;
}
Flexfloat::mtype Flexfloat::max_mant() const
{
    return (1 << M) - 1u;
}
Flexfloat::etype Flexfloat::max_exp(Etype E)
{
    return (1 << E) - 1;
}
Flexfloat::mtype Flexfloat::max_mant(Mtype M)
{
    return (1 << M) - 1u;
}

bool Flexfloat::is_zero(const Flexfloat &val)
{
    if (val.m == 0 && val.e == 0)
        return true;
    else
        return false;
}

Flexfloat &Flexfloat::operator=(const Flexfloat &other)
{
    // assert(B == other.B);
    // assert(E == other.E);

    // if (other.M >= M) {
    //     m = other.m >> (other.M - M);
    // }
    // if (other.M < M) {
    //     m = other.m << (M - other.M);
    // }

    e = other.e;
    s = other.s;
    m = other.m;

    E = other.E;
    M = other.M;
    B = other.B;

    return *this;
}

void Flexfloat::check_ffs(std::initializer_list<Flexfloat> list)
{
    for (auto &elem : list)
    {
        if (!elem.is_valid())
        {
            throw std::runtime_error{"Invalid operand"};
        }
    }
}

void Flexfloat::mult(const Flexfloat &lhs, const Flexfloat &rhs, Flexfloat &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "Multiplication of two numbers";
    check_ffs({lhs, rhs, res});
    CLOG(trace) << "Left  operand: " << lhs;
    CLOG(trace) << "Right operand: " << rhs;
#endif

    // Corner cases
    if (is_zero(lhs) || is_zero(rhs))
    {
        res.e = 0;
        res.m = 0;
        res.s = lhs.s ^ rhs.s;
        return;
    }

    // rhs.s and lhs.s can only be 0 or 1
    stype nsign = static_cast<stype>(lhs.s ^ rhs.s);
    eexttype nexp = lhs.e + rhs.e + res.B - lhs.B - rhs.B;

    // Largest M. All calculations will be with the largest mantissa
    Mtype LM = std::max({lhs.M, rhs.M, res.M});

    mexttype lhs_m = unzip(lhs);
    mexttype rhs_m = unzip(rhs);

    // Casting all mantissa's to LM
    lhs_m <<= (LM - lhs.M);
    rhs_m <<= (LM - rhs.M);

    mexttype nmant = lhs_m * rhs_m;

    res = normalise(nsign, nexp - LM, nmant, LM, {res.E, res.M, res.B});

    $(CLOG(trace) << "Result: " << res);
    return;
}

std::ostream &operator<<(std::ostream &oss, const Flexfloat &num)
{
    oss << "(E, M, B) = (" << +num.E << ", " << +num.M << ", " << +num.B << ")";
    oss << "  val = " << num.bits();

    return oss;
}

void Flexfloat::sum(const Flexfloat &lhs, const Flexfloat &rhs, Flexfloat &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "Sum of two numbers";
    check_ffs({lhs, rhs, res});
    CLOG(trace) << "Left  operand: " << lhs;
    CLOG(trace) << "Right operand: " << rhs;
#endif

    mexttype lhs_m = unzip(lhs);
    mexttype rhs_m = unzip(rhs);

    // Largest M. All calculations will be with the largest mantissa
    Mtype LM = std::max({lhs.M, rhs.M, res.M});

    // Casting all mantissa's to LM
    lhs_m <<= (LM - lhs.M);
    rhs_m <<= (LM - rhs.M);

    eexttype rhs_e = rhs.e;
    eexttype lhs_e = lhs.e;

    // Casting inputs to maximum exponent
    eexttype nexp = 0;
    if (lhs_e - lhs.B > rhs_e - rhs.B)
    {
        auto delta_e = lhs_e - rhs_e - lhs.B + rhs.B;
        rhs_e += delta_e;
        rhs_m >>= delta_e;

        assert(lhs_e - lhs.B + res.B >= 0);
        nexp = lhs_e - lhs.B + res.B;
    }
    else
    {
        auto delta_e = rhs_e - lhs_e - rhs.B + lhs.B;
        lhs_e += delta_e;
        lhs_m >>= delta_e;

        assert(rhs_e - rhs.B + res.B >= 0);
        nexp = rhs_e - rhs.B + res.B;
    }

#ifdef BOOST_LOGS
    CLOG(trace) << "=================== Values after casting ====================";
    CLOG(trace) << "lhs_e:  " << clib::bits(lhs_e);
    CLOG(trace) << "lhs_m:  " << clib::bits(lhs_m);
    CLOG(trace) << "rhs_e: " << clib::bits(rhs_e);
    CLOG(trace) << "rhs_m: " << clib::bits(rhs_m);
    CLOG(trace) << "=============================================================";
#endif

    mexttype nmant = 0;
    stype nsign = 0;

    if (lhs.s == rhs.s)
    {
        nsign = lhs.s;
        nmant = lhs_m + rhs_m;
    }
    else if (lhs_m >= rhs_m)
    {
        nsign = lhs.s;
        nmant = lhs_m - rhs_m;
    }
    else
    {
        nsign = rhs.s;
        nmant = rhs_m - lhs_m;
    }

    res = normalise(nsign, nexp, nmant, LM, {res.E, res.M, res.B});

    $(CLOG(trace) << "Result: " << res);
    return;
}

void Flexfloat::sub(const Flexfloat &lhs, const Flexfloat &rhs, Flexfloat &res)
{
    sum(lhs, Flexfloat(rhs.E, rhs.M, rhs.B, !rhs.s, rhs.e, rhs.m), res);
}

Flexfloat::ext_ff Flexfloat::get_normalized(const Flexfloat &denorm)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "Getting normal value of denormal";
    check_ffs({denorm});
    CLOG(trace) << "Value " << denorm;

    assert(denorm.e == 0);
#endif

    mexttype ext_mant = denorm.m;
    if (denorm.m == 0)
        ext_mant = 1;

    auto N = msb(ext_mant);
    auto n = ext_mant - (1 << msb(ext_mant));
    auto delta_N = denorm.M - N;

    eexttype ext_exp = 1 - delta_N;
    ext_mant = n << delta_N;

#ifdef BOOST_LOGS
    CLOG(trace) << "Extended exp  = " << clib::bits(ext_exp);
    CLOG(trace) << "Extended mant = " << clib::bits(ext_mant);
#endif

    return ext_ff{ext_exp, ext_mant};
}

void Flexfloat::inv(const Flexfloat &x, Flexfloat &res, size_t precision)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "Inv: 1/x";
    check_ffs({x, res});
    CLOG(trace) << "x: " << x;
#endif

    eexttype nexp = x.e;
    mexttype nmant = x.m;

    if (nmant == 0 && nexp == 0)
    {
        res = res.max_norm();
        res.s = x.s;
        return;
    }
    if (nexp == 0)
    {
        ext_ff normal = get_normalized(x);
        nexp = normal.exp;
        nmant = normal.mant;
    }
    if (nmant == 0)
    {
        nexp = -nexp + x.B + res.B;
        nmant = 0;
    }

    else if (precision == 0)
    {
        nexp = -nexp + (x.B + res.B - 1);
        nmant = (1 << x.M) - nmant - 1;
    }
    else
    {
        nexp = -nexp + (x.B + res.B - 1);
        assert(nmant < std::numeric_limits<polyfit_t>::max());
        nmant = polyfit::get()->calc("inv", static_cast<polyfit_t>(nmant), x.M, res.M);
    }

    // normalise expects extended mantissa
    nmant += 1 << res.M;
    res = normalise(x.s, nexp, nmant, res.M, {res.E, res.M, res.B});
}

void Flexfloat::exp2(const Flexfloat &x, Flexfloat &res, uint8_t F)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "exp(x)";
    check_ffs({x, res});
    CLOG(trace) << "x: " << x;
#endif

    eexttype intp = x.integer_part();
    auto frac = x.fractional_part(F);

#ifdef BOOST_LOGS
    CLOG(trace) << "integer_part = " << intp;
    CLOG(trace) << "fractional_part = " << frac;
#endif

    // negative
    if (x.s == 1)
    {
        intp = -(intp + 1);
        frac = ~frac & ((1u << F) - 1u);
#ifdef BOOST_LOGS
        CLOG(trace) << "x is negative!";
        CLOG(trace) << "integer_part = " << intp;
        CLOG(trace) << "fractional_part = " << frac;
#endif
    }

    auto nmant = polyfit::get()->calc("exp2", frac, F, res.M);
    nmant += 1 << res.M;
    res = normalise(0, intp + res.B, nmant, res.M, {res.E, res.M, res.B});
}

void Flexfloat::log2(const Flexfloat &x, Flexfloat &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "log2(x)";
    check_ffs({x, res});
    CLOG(trace) << "x: " << x;
#endif
    eexttype nexp = x.e;
    mexttype nmant = x.m;

    if (nexp == 0)
    {
        ext_ff normal = get_normalized(x);
        nexp = normal.exp;
        nmant = normal.mant;
    }

    if (x.s == 1)
        std::runtime_error{"It is impossible to take the logarithm of a negative number"};

    nexp = nexp - x.B - 1;
    auto a = from_arithmetic_t(res, nexp);

    nmant = polyfit::get()->calc("log2", nmant, x.M, res.M);
    auto b = Flexfloat(res, nmant);
    b.e += b.B;

    // sum of 2 Flexfloat
    sum(a, b, res);
}

void Flexfloat::sqrt(const Flexfloat &x, Flexfloat &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "sqrt(x)";
    check_ffs({x, res});
    CLOG(trace) << "x: " << x;
#endif
    if (x.s == 1)
        std::runtime_error{"It is impossible to take the sqrt of a negative number"};

    eexttype nexp = x.e;
    mexttype nmant = x.m;

    if (nexp == 0)
    {
        ext_ff normal = get_normalized(x);
        nexp = normal.exp;
        nmant = normal.mant;
    }

    auto eps = nexp - x.B;
    auto n = eps / 2;
    auto k = eps % 2;

    if (k < 0)
    {
        k += 2;
        n -= 1;
    }

    if (k == 0)
    {
        $(CLOG(trace) << "k == 0");
        nmant = polyfit::get()->calc("sqrt", nmant, x.M, res.M);
        nmant += 1 << res.M;
        res = normalise(0, n + res.B, nmant, res.M, {res.E, res.M, res.B});
        return;
    }

    $(CLOG(trace) << "k != 0");
    nmant = polyfit::get()->calc("sqrt2", nmant, x.M, res.M);
    nmant += 1 << res.M;
    res = normalise(0, n + res.B, nmant, res.M, {res.E, res.M, res.B});
}

void Flexfloat::cos(const Flexfloat &x, Flexfloat &res, uint8_t F)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "cos(x)";
    check_ffs({x, res});
    CLOG(trace) << "x: " << x;
#endif

    auto pi_2 = from_arithmetic_t(x, static_cast<float>(1.0 / (3.141592653589793 / 2.0)));
    Flexfloat y = x;
    y.s = 0;
    mult(y, pi_2, y);

    auto n = y.integer_part();
    auto frac = y.fractional_part(F);

#ifdef BOOST_LOGS
    CLOG(trace) << "y = " << y.to_float();
    CLOG(trace) << "integer_part = " << n;
    CLOG(trace) << "fractional_part = " << frac;
#endif

    if (n % 2 != 0)
        frac = (1 << F) - frac;

    uint8_t cos_sign;    
    if (n % 4 == 0 or n % 4 == 3)
        cos_sign = 0;
    else
        cos_sign = 1;

    auto fx_val = polyfit::get()->calc("cos", frac, F, F);
    $(CLOG(trace) << "fx_val = " << fx_val);

    Flexfixed fx(1, F, cos_sign, fx_val);
    to_flexfloat(fx, res);
}

void Flexfloat::sin(const Flexfloat &x, Flexfloat &res, uint8_t F)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "sin(x)";
    check_ffs({x, res});
    CLOG(trace) << "x: " << x;
#endif

    uint8_t x_sign = x.s;
    auto y = x;
    y.s = 0;

    auto pi_2 = from_arithmetic_t(x, static_cast<float>(1.0 / (3.141592653589793 / 2.0)));
    mult(y, pi_2, y);

    auto n = y.integer_part();
    auto frac = y.fractional_part(F);

#ifdef BOOST_LOGS
    CLOG(trace) << "y = " << y.to_float();
    CLOG(trace) << "integer_part = " << n;
    CLOG(trace) << "fractional_part = " << frac;
#endif

    if (n % 2 != 0)
        frac = (1 << F) - frac;

    uint8_t sin_sign;
    if (n % 4 == 0 or n % 4 == 1)
        sin_sign = 0;
    else
        sin_sign = 1;

    auto fx_val = polyfit::get()->calc("sin", frac, F, F);
    $(CLOG(trace) << "fx_val = " << fx_val);

    Flexfixed fx(1, F, x_sign ^ sin_sign, fx_val);
    to_flexfloat(fx, res);
}

void Flexfloat::ctan(const Flexfloat &x, Flexfloat &res, uint8_t F)
{
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "ctan(x)";
    check_ffs({x, res});
    CLOG(trace) << "x: " << x;
#endif

    const uint32_t f_pi_2 = 636619772;

    auto y = x;
    y.s = 0;

    auto inv_pi = from_arithmetic_t(x, static_cast<float>(1.0 / (3.141592653589793)));
    auto pi = from_arithmetic_t(x, static_cast<float>(3.141592653589793));
    mult(y, inv_pi, y);

    auto n = y.integer_part();
    auto frac = y.fractional_part(F);

#ifdef BOOST_LOGS
    CLOG(trace) << "y = " << y.to_float();
    CLOG(trace) << "integer_part = " << n;
    CLOG(trace) << "fractional_part = " << frac;
#endif

    // if frac/F > 1/2
    auto adj_frac = frac;
    uint8_t ctan_sign = 0;
    if (adj_frac > (1u << (F-1u)))
    {
        $(CLOG(trace) << "ctan_sign = 0 ");
        adj_frac = (1 << F) - adj_frac;
        ctan_sign = 1;
    }
    
    // frac *= 2
    adj_frac <<= 1;
    
    auto fx_val = polyfit::get()->calc("ctan", adj_frac, F, F);
    $(CLOG(trace) << "fx_val = " << fx_val);

    fx_val = (fx_val * f_pi_2) >> (msb(f_pi_2) + 1);
    $(CLOG(trace) << "fx_val * f_pi_2 = " << fx_val);
    
    Flexfixed fx(1, F, 1, fx_val);
    to_flexfloat(fx, res);

    $(CLOG(trace) << "f(x) = " << res.to_float());

    auto inv_x = res;
    Flexfixed inv_fx(1, F, 0, adj_frac >> 1);
    to_flexfloat(inv_fx, inv_x);
    mult(pi, inv_x, inv_x);
    inv(inv_x, inv_x, 1);

    $(CLOG(trace) << "1/x = " << inv_x.to_float());

    sum(res, inv_x, res);
    res.s = x.s ^ ctan_sign;
}

int Flexfloat::ceil() const
{
#ifdef BOOST_LOGS
    CLOG(trace) << "ceil";
    check_ffs({*this});
    CLOG(trace) << "ff: " << *this;
#endif

    int sign = (s == 1) ? -1 : 1;
    int nexp = e + B - M;
    mexttype nmant = m;

    // denormal Flexfloat
    if (e == 0)
        nmant *= 2;
    else
        nmant += (1 << M);

    auto nval = nmant;
    if (nexp < 0)
        // Округление вниз
        nval = (nmant >> -nexp);
    if (nexp > 0)
        nval = (nmant << nexp);

    if (nval > std::numeric_limits<int>::max())
    {
        std::cout << *this << std::endl;
        throw std::runtime_error{"Flexlfloat can not fit to int: ff > max(int)"};
        return std::numeric_limits<int>::max() * sign;
    }
    return static_cast<int>(nval) * sign;
}

#pragma GCC diagnostic ignored "-Wstrict-overflow"
float Flexfloat::to_float() const
{
    eexttype nexp = e;
    mexttype nmant = m;
    if (e == 0)
    {
        ext_ff normal = get_normalized(*this);
        nexp = normal.exp;
        nmant = normal.mant;
    }
    // get extended mantissa
    nmant += 1 << M;

    // CONVERAION
    nexp = nexp - B + B_FLOAT;
    if (M_FLOAT > M)
        nmant = nmant * (1 << (M_FLOAT - M));
    else
        nmant = nmant / (1 << (M - M_FLOAT));

#ifdef BOOST_LOGS
    CLOG(dump) << std::endl;
    CLOG(dump) << "FlexFloat to_float";
    CLOG(dump) << *this;
    CLOG(dump) << "exp after conversion  = " << clib::bits(nexp);
    CLOG(dump) << "mant after conversion = " << clib::bits(nmant);
#endif

    Flexfloat ans = normalise(s, nexp, nmant, M_FLOAT, {E_FLOAT, M_FLOAT, B_FLOAT});

    ieee754_float fc;
    assert(ans.e <= 1 << E_FLOAT);
    assert(ans.s <= 1);
    assert(ans.m <= 1 << M_FLOAT);

#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
    fc.ieee.negative = ans.s;
    fc.ieee.exponent = ans.e;
    fc.ieee.mantissa = ans.m;
#pragma GCC diagnostic warning "-Wconversion"
#pragma GCC diagnostic warning "-Wsign-conversion"

    return fc.f;
}
#pragma GCC diagnostic warning "-Wstrict-overflow"

int Flexfloat::to_int() const
{
    int sign = (s == 1) ? -1 : 1;
    auto nexp = e - B - M;
    mexttype nmant = m;

    // denormal Flexfloat
    if (e == 0)
        nmant *= 2;
    else
        nmant += (1 << M);

    auto nval = nmant;
    if (nexp < 0)
    {
        nval = (nmant >> -nexp);
        // Округление по математическим законами
        if ((nmant >> -(nexp + 1)) % 2 == 1)
            nval += 1;
    }
    if (nexp > 0)
        nval = (nmant << nexp);

    if (nval > std::numeric_limits<int>::max())
    {
        throw std::runtime_error{"Flexlfloat can not fit to int: ff > max(int)"};
        return std::numeric_limits<int>::max() * sign;
    }
    return static_cast<int>(nval) * sign;
}

uint32_t Flexfloat::integer_part() const
{
    $(CLOG(trace) << "integer_part");
    eexttype nexp = e;
    mexttype nmant = m;

    if (nexp == 0)
    {
        ext_ff normal = get_normalized(*this);
        nexp = normal.exp;
        nmant = normal.mant;
    }
    nexp -= B;

    if (nexp < 0)
        return 0;
    if (nexp == 0)
        return 1;

    auto m0 = m >> (M - nexp);

    $(CLOG(trace) << "m0 = " << m0);

    if (nexp <= M)
    {
        auto ans = (1 << nexp) + m0;
        if (std::numeric_limits<uint32_t>::max() < ans)
            throw std::runtime_error{"Can not fit Flexfloat in int"};
        return ans;
    }

    auto ans = (1 << (nexp - M)) * ((1 << M) + nmant);
    if (std::numeric_limits<uint32_t>::max() < ans)
        throw std::runtime_error{"Can not fit Flexfloat in int"};
    return static_cast<uint32_t>(ans);
}

Flexfixed Flexfloat::to_flexfixed(uint8_t I, uint8_t F) const
{
    auto temp_ff = *this;
    temp_ff.e += F;

    auto n = integer_part();
    auto int_part = n >> F;
    if (msb(int_part) + 1 >= (1 << I))
        throw std::runtime_error{"Can not fit Flexfloat in Flexfixed"};

    return Flexfixed(I, F, s, n);
}

uint32_t Flexfloat::fractional_part(uint8_t F) const
{
    $(CLOG(trace) << "fractional_part");
    auto temp_ff = *this;
    temp_ff.e += F;

    auto n = temp_ff.integer_part();
    auto frac_part = n & ((1u << F) - 1u);
    assert(frac_part < std::numeric_limits<uint32_t>::max());

    return static_cast<uint32_t>(frac_part);
}

Flexfloat Flexfloat::from_arithmetic_t(Etype E, Mtype M, Btype B, float flt)
{
    ieee754_float fc;
    fc.f = flt;

#pragma GCC diagnostic ignored "-Wconversion"
    stype nsign = fc.ieee.negative;
    eexttype nexp = fc.ieee.exponent;
    mexttype nmant = fc.ieee.mantissa;
#pragma GCC diagnostic warning "-Wconversion"
    // get extended mantissa
    nmant += 1 << M_FLOAT;

    // CONVERAION
    nexp = nexp - B_FLOAT + B;
    if (M_FLOAT > M)
        nmant = nmant / (1 << (M_FLOAT - M));
    else
        nmant = nmant * (1 << (M - M_FLOAT));

#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "FlexFloat from_float = " << flt;
    CLOG(trace) << "exp after conversion  = " << clib::bits(nexp);
    CLOG(trace) << "mant after conversion = " << clib::bits(nmant);
#endif

    return normalise(nsign, nexp, nmant, M, {E, M, B});
}

Flexfloat Flexfloat::from_arithmetic_t(const Flexfloat &hyperparams, float flt)
{
    return from_arithmetic_t(hyperparams.E, hyperparams.M, hyperparams.B, flt);
}

void Flexfloat::from_arithmetic_t(float flt, const Flexfloat &in, Flexfloat &out)
{
    out = from_arithmetic_t(in.E, in.M, in.B, flt);
}

Flexfloat Flexfloat::from_arithmetic_t(Etype E, Mtype M, Btype B, int n)
{
    $(CLOG(trace) << "FlexFloat from int = " << n);

    if (n == 0)
    {
        return Flexfloat(E, M, B, 0);
    }

    stype s = 0;
    if (n < 0)
    {
        n = -n;
        s = 1;
    }
    unsigned n_uns = static_cast<unsigned>(n);

    Mtype N = msb(n_uns);
    mexttype mant = 0;
    mexttype delta_N = n_uns - (1 << N);
    if (N > M)
        mant = delta_N >> (N - M);
    else
        mant = delta_N << (M - N);

    // normalise expects extended mantissa
    mant += 1 << M;
    return normalise(s, N + B, mant, M, {E, M, B});
}
Flexfloat Flexfloat::from_arithmetic_t(const Flexfloat &hyperparams, int n)
{
    return from_arithmetic_t(hyperparams.E, hyperparams.M, hyperparams.B, n);
}
void Flexfloat::from_arithmetic_t(int n, const Flexfloat &in, Flexfloat &out)
{
    out = from_arithmetic_t(in.E, in.M, in.B, n);
}

Flexfloat Flexfloat::from_arithmetic_t(Etype E, Mtype M, Btype B, long unsigned n)
{
    $(CLOG(trace) << "FlexFloat from long unsigned = " << n);

    if (n == 0)
    {
        return Flexfloat(E, M, B, 0);
    }

    stype s = 0;

    Mtype N = msb(n);
    mexttype mant = 0;
    mexttype delta_N = n - (1 << N);
    if (N > M)
        mant = delta_N >> (N - M);
    else
        mant = delta_N << (M - N);

    // normalise expects extended mantissa
    mant += 1 << M;
    return normalise(s, N + B, mant, M, {E, M, B});
}
Flexfloat Flexfloat::from_arithmetic_t(const Flexfloat &hyperparams, long unsigned n)
{
    return from_arithmetic_t(hyperparams.E, hyperparams.M, hyperparams.B, n);
}
void Flexfloat::from_arithmetic_t(long unsigned n, const Flexfloat &in, Flexfloat &out)
{
    out = from_arithmetic_t(in.E, in.M, in.B, n);
}

Flexfloat Flexfloat::from_arithmetic_t(Etype E, Mtype M, Btype B, int64_t n)
{
    $(CLOG(trace) << "FlexFloat from int = " << n);

    if (n == 0)
    {
        return Flexfloat(E, M, B, 0);
    }

    stype s = 0;
    if (n < 0)
    {
        n = -n;
        s = 1;
    }
    uint64_t n_uns = static_cast<uint64_t>(n);

    Mtype N = msb(n_uns);
    mexttype mant = 0;
    mexttype delta_N = n_uns - (1 << N);
    if (N > M)
        mant = delta_N >> (N - M);
    else
        mant = delta_N << (M - N);

    // normalise expects extended mantissa
    mant += 1 << M;
    return normalise(s, N + B, mant, M, {E, M, B});
}
Flexfloat Flexfloat::from_arithmetic_t(const Flexfloat &hyperparams, int64_t n)
{
    return from_arithmetic_t(hyperparams.E, hyperparams.M, hyperparams.B, n);
}
void Flexfloat::from_arithmetic_t(int64_t n, const Flexfloat &in, Flexfloat &out)
{
    out = from_arithmetic_t(in.E, in.M, in.B, n);
}

std::string Flexfloat::bits() const
{
    std::stringstream ostream;

    ostream << std::bitset<1>(s) << "|" << to_string_e() << "|" << to_string_m();

    return ostream.str();
}

std::string Flexfloat::bits(const Flexfloat &ff) const
{
    std::stringstream ostream;

    ostream << std::bitset<1>(s) << "|" << std::setw(ff.E) << to_string_e() << "|" << to_string_m() << std::setw(ff.M);

    return ostream.str();
}

bool operator>(const Flexfloat &lhs, const Flexfloat &rhs)
{
    assert(lhs.E == rhs.E && lhs.B == rhs.B && lhs.M == rhs.M);
#ifdef BOOST_LOGS
    CLOG(trace) << "operator>";
    Flexfloat::check_ffs({lhs, rhs});
    CLOG(trace) << "lhs: " << lhs;
    CLOG(trace) << "rhs: " << rhs;
#endif

    if (lhs.s != rhs.s)
        return lhs.s == 0;

    // Largest M. All calculations will be with the largest mantissa
    Flexfloat::Mtype LM = std::max({lhs.M, rhs.M});

    Flexfloat::mexttype lhs_m = Flexfloat::unzip(lhs);
    Flexfloat::mexttype rhs_m = Flexfloat::unzip(rhs);

    // Casting all mantissa's to LM
    lhs_m <<= (LM - lhs.M);
    rhs_m <<= (LM - rhs.M);

    auto lhs_e = lhs.e + msb(lhs_m) - lhs.B;
    auto rhs_e = rhs.e + msb(rhs_m) - rhs.B;

    if (lhs_e > rhs_e)
        return lhs.s == 0;
    if (lhs_e < rhs_e)
        return lhs.s == 1;

    // Exponents are equal
    if (lhs_m > rhs_m)
        return lhs.s == 0;

    if (lhs_m == rhs_m)
        return false;

    if (lhs_m < rhs_m)
        return lhs.s == 1;

    assert(0);
    return 0;
}
bool operator==(const Flexfloat &lhs, const Flexfloat &rhs)
{
    assert(lhs.E == rhs.E && lhs.B == rhs.B && lhs.M == rhs.M);
    return lhs.s == rhs.s && lhs.m == rhs.m && lhs.e == rhs.e;
}

bool operator<(const Flexfloat &lhs, const Flexfloat &rhs)
{
    return !(lhs > rhs);
}
bool operator>=(const Flexfloat &lhs, const Flexfloat &rhs)
{
    return (lhs > rhs) || (lhs == rhs);
}
bool operator<=(const Flexfloat &lhs, const Flexfloat &rhs)
{
    return !(lhs > rhs) || (lhs == rhs);
}
bool operator!=(const Flexfloat &lhs, const Flexfloat &rhs)
{
    return !(lhs == rhs);
}

void Flexfloat::min(const Flexfloat &first, const Flexfloat &second, Flexfloat &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "min";
    Flexfloat::check_ffs({first, second, res});
    CLOG(trace) << "first: " << first;
    CLOG(trace) << "second: " << second;
#endif
    if (first > second)
        res = second;
    else
        res = first;
}
void Flexfloat::max(const Flexfloat &first, const Flexfloat &second, Flexfloat &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "max";
    Flexfloat::check_ffs({first, second, res});
    CLOG(trace) << "first: " << first;
    CLOG(trace) << "second: " << second;
#endif
    if (first < second)
        res = second;
    else
        res = first;
}

void Flexfloat::clip(const Flexfloat &a, const Flexfloat &x, const Flexfloat &b, Flexfloat &out)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "clip";
    Flexfloat::check_ffs({a, x, b, out});
    CLOG(trace) << "a: " << a;
    CLOG(trace) << "x: " << x;
    CLOG(trace) << "b: " << b;
#endif

    min(x, b, out);
    max(a, out, out);
}

void to_flexfloat(const Flexfixed &value, Flexfloat &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "from_fx_to_ff";
    Flexfloat::check_ffs({res});
    CLOG(trace) << "value: " << value;
#endif

    res.s = value.s;

    if (value.get_n() == 0)
    {
        res.e = 0;
        res.m = 0;
        return;
    }

    Flexfixed::wtype msb = Flexfixed::msb(value);

    assert(res.max_exp() > msb + res.B - value.get_F());
    res.e = msb + res.B - value.get_F();

    assert(res.e < res.max_exp());

    Flexfixed::wtype delta = msb - res.M;

    if (delta >= 0)
    {
        assert(res.max_mant() >= (value.get_n() - (1 << msb)) >> delta);
        res.m = static_cast<Flexfloat::mtype>((value.get_n() - (1 << msb)) >> delta);
    }
    else
    {
        assert(res.max_mant() >= ((value.get_n() - (1 << msb)) << -delta));
        res.m = static_cast<Flexfloat::mtype>((value.get_n() - (1 << msb)) << -delta);
    }

    assert(res.m < res.max_mant());

    $(CLOG(trace) << "res: " << res);
}

//
// See
// gitlab.inviewlab.com/synthesizer/documents/-/blob/master/out/flexfloat_normalize.pdf
//
Flexfloat Flexfloat::normalise(stype cur_sign, eexttype cur_exp, mexttype cur_mant, Mtype curM, hyper_params res)
{
#ifdef BOOST_LOGS
    CLOG(dump) << "============== Values before normalisation ==================";
    CLOG(dump) << "exp:  " << clib::bits(cur_exp);
    CLOG(dump) << "mant: " << clib::bits(cur_mant);
    CLOG(dump) << "=============================================================";
#endif

    if (cur_exp > 0 && cur_mant == 0)
    {
        cur_exp -= 1;
        cur_mant = 1;
    }

    auto rshift = [&cur_exp, &cur_mant](eexttype n) {
        assert(n >= 0);
        $(CLOG(dump) << "rshift on n = " << static_cast<uint64_t>(n));
        if (n > msb(cur_mant))
            cur_mant = 0;
        else
            cur_mant = cur_mant >> n;
        cur_exp = n + cur_exp;
    };
    auto lshift = [&cur_exp, &cur_mant](eexttype n) {
        assert(n >= 0);
        $(CLOG(dump) << "lshift on n = " << static_cast<uint64_t>(n));
        cur_mant = cur_mant << n;
        cur_exp = cur_exp - n;
    };

    auto ovf = [&cur_exp, &cur_mant, curM, res]() {
        $(CLOG(dump) << "overflow");
        cur_mant = max_mant(curM + 1);
        cur_exp = max_exp(res.E);
    };
    auto unf = [&cur_exp, &cur_mant, curM, res]() {
        $(CLOG(dump) << "underflow");
        cur_mant = 0;
        cur_exp = 0;
    };

    eexttype delta_m = (cur_mant > 0) ? std::abs(msb(cur_mant) - curM) : 0;
    eexttype delta_e = std::abs(cur_exp - max_exp(res.E));

    if (cur_exp <= 0)
    {
        // We must decrease mantissa, unless exponent != 0
        $(CLOG(dump) << "cur_exp <= 0");

        if (cur_mant == 0)
        {
            $(CLOG(dump) << "cur_mant = 0");
            unf();
        }
        else if (msb(cur_mant) <= curM)
        {
            $(CLOG(dump) << "msb(cur_mant) <= curM");
            rshift(-cur_exp);
        }
        else
        {
            $(CLOG(dump) << "msb(cur_mant) > curM");
            if (delta_m > delta_e)
                ovf();
            else
                rshift(std::max(-cur_exp, delta_m));
        }
    }
    else if (cur_exp > 0 && cur_exp <= max_exp(res.E))
    {
        // Exponent is normal. We must normalise mantissa
        $(CLOG(dump) << "cur_exp > 0 && cur_exp <= max_exp(res.E)");

        if (msb(cur_mant) <= curM)
        {
            $(CLOG(dump) << "msb(cur_mant) <= curM");
            lshift(std::min(cur_exp, delta_m));
        }
        else
        {
            $(CLOG(dump) << "msb(cur_mant) > curM");
            if (delta_m > delta_e)
                ovf();
            else
                rshift(delta_m);
        }
    }
    else
    {
        // Exponent is big. We must decrease it, unless exponent > max_exp
        $(CLOG(dump) << "cur_exp > max_exp(res.E)");

        if (msb(cur_mant) <= curM)
        {
            $(CLOG(dump) << "msb(cur_mant) <= curM");

            if (delta_m < delta_e)
                ovf();
            else
                lshift(std::min(delta_m, cur_exp));
        }
        else
        {
            $(CLOG(dump) << "msb(cur_mant) > curM");
            ovf();
        }
    }

    assert(res.E < sizeof(etype) * 8);
    assert(curM < sizeof(mtype) * 8);
    assert(msb(cur_mant) <= sizeof(mtype) * 8);

    mtype mant = zip(cur_exp, cur_mant, curM, res.M);

    assert(cur_exp >= 0);
    assert(cur_exp <= max_exp(res.E));
    etype exp = static_cast<etype>(cur_exp);

#ifdef BOOST_LOGS
    CLOG(dump) << "================ Values after normalisation =================";
    CLOG(dump) << "exp:  " << clib::bits(cur_exp);
    CLOG(dump) << "mant: " << clib::bits(cur_mant);
    CLOG(dump) << "=============================================================";
    CLOG(dump) << "===================== Values after zip ======================";
    CLOG(dump) << "exp:  " << clib::bits(exp);
    CLOG(dump) << "mant: " << clib::bits(mant);
    CLOG(dump) << "=============================================================";
#endif

    return Flexfloat(res.E, res.M, res.B, cur_sign, exp, mant);
}

void Flexfloat::negative(const Flexfloat &val, Flexfloat &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "negative";
    Flexfloat::check_ffs({val, res});
    CLOG(trace) << "Value: " << val;
    CLOG(trace) << "Result: " << res;
#endif

    assert(val.E == res.E);
    assert(val.M == res.M);

    res.e = val.e;
    res.m = val.m;
    res.s = val.s >= 1 ? 0 : 1;
    $(CLOG(trace) << "res: " << res);
}

// if e > 0  -> normalized value   -> m' = 2^M + m
// if e == 0 -> denormalized value -> m' = 2*m
Flexfloat::mtype Flexfloat::zip(eexttype exp, mexttype ext_mant, Mtype curM, Mtype reqM)
{
    if (exp > 0)
    {
        $(CLOG(dump) << "exp > 0: Value is normalized");

        assert(ext_mant >= max_mant(curM));
        ext_mant -= max_mant(curM) + 1;
    }
    else
    {
        $(CLOG(dump) << "exp == 0: Value is denormalized");

        ext_mant >>= 1;
    }

    $(CLOG(dump) << "ext_mant = " << clib::bits(ext_mant));

    if (curM >= reqM)
    {
        $(CLOG(dump) << "curM >= reqM");

        ext_mant = ext_mant >> (curM - reqM);
    }
    else
    {
        $(CLOG(dump) << "curM < reqM");

        ext_mant = ext_mant << (reqM - curM);
    }

    assert(ext_mant <= max_mant(reqM));
    return static_cast<mtype>(ext_mant);
}

void Flexfloat::abs(const Flexfloat &val, Flexfloat &res)
{
#ifdef BOOST_LOGS
    CLOG(trace) << "abs";
    Flexfloat::check_ffs({val, res});
    CLOG(trace) << "Value: " << val;
    CLOG(trace) << "Result: " << res;
#endif

    assert(val.E == res.E);
    assert(val.M == res.M);

    res.e = val.e;
    res.m = val.m;
    res.s = 0;
    $(CLOG(trace) << "res: " << res);
}

// if e > 0  -> normalized value   -> m' = 2^M + m
// if e == 0 -> denormalized value -> m' = 2*m
Flexfloat::mtype Flexfloat::zip(eexttype exp, mexttype ext_mant, Mtype M)
{
    if (exp > 0)
    {
        $(CLOG(trace) << "exp > 0: Value is normalized");

        assert(ext_mant >= max_mant(M));
        ext_mant -= max_mant(M) + 1;
    }
    else
    {
        $(CLOG(trace) << "exp == 0: Value is denormalized");

        ext_mant >>= 1;
    }

    assert(ext_mant <= max_mant(M));
    return static_cast<mtype>(ext_mant);
}

// if e > 0  -> normalized value   -> m' = 2^M + m
// if e == 0 -> denormalized value -> m' = 2*m
Flexfloat::mexttype Flexfloat::unzip(etype exp, mtype mant, Mtype M)
{
    if (exp == 0)
        return (2 * mant);
    else
        return ((1 << M) + mant);
}
Flexfloat::mexttype Flexfloat::unzip(const Flexfloat &ff)
{
    if (ff.e == 0)
        return (2 * ff.m);
    else
        return ((1 << ff.M) + ff.m);
}

bool Flexfloat::is_valid() const
{
    if (M > sizeof(m) * 8)
    {
        $(CLOG(error) << "M > sizeof(m)*8");
        return false;
    }
    if (E > sizeof(e) * 8)
    {
        $(CLOG(error) << "E > sizeof(e)*8");
        return false;
    }
    if (s > 1)
    {
        $(CLOG(error) << "s > 1");
        return false;
    }
    if (e > (1 << E) - 1)
    {
        $(CLOG(error) << "e > ((1 << E) - 1)");
        return false;
    }
    if (m > (1u << M) - 1u)
    {
        $(CLOG(error) << "m > ((1 << M) - 1)");
        return false;
    }

    return true;
}

} // namespace clib
