#include <clib/Flexfloat.hpp>

#include "clib/logs.hpp"

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

Flexfloat::Flexfloat(Etype E_n, Mtype M_n, Btype B_n, uint32_t value) : B(B_n), E(E_n), M(M_n), s(0), e(0), m(0)
{
    m = (value >> 0) & ((1u << M) - 1u);
    
    assert(msb(value) + 1 <= M + B + S);
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

Flexfloat::Flexfloat(const Flexfloat &hyperparams, uint32_t value) : B(0), E(0), M(0), s(0), e(0), m(0)
{
    B = hyperparams.get_B();
    E = hyperparams.get_E();
    M = hyperparams.get_M();

    m = (value >> 0) & ((1u << M) - 1u);

    assert(msb(value) + 1 <= M + B + S);
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

Flexfloat Flexfloat::pack(const Flexfloat& in, hyper_params req_hyperparams)
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

Flexfloat::Mtype Flexfloat::msb(mexttype val)
{
    if (val == 0)
        return 0;

    Mtype msb = 0;
    while (val > 0)
    {
        val >>= 1;
        msb++;
    }

    return msb - 1;
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

    Flexfloat norm_ans = normalise(nsign, nexp - LM, nmant, LM, {res.E, res.M, res.B});
    res = norm_ans;

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

    Flexfloat norm_ans = normalise(nsign, nexp, nmant, LM, {res.E, res.M, res.B});
    res = norm_ans;

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

void Flexfloat::inv(const Flexfloat &x, Flexfloat &res)
{
    size_t precision = 0; // TODO
#ifdef BOOST_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "Inv: 1/x";
    check_ffs({x, res});
    CLOG(trace) << "x: " << x;
#endif

    if (x.m == 0 && x.e == 0)
    {
        res = res.max_norm();
        res.s = x.s;
        return;
    }

    eexttype nexp = x.e;
    mexttype nmant = x.m;
    if (x.e == 0)
    {
        ext_ff normal = get_normalized(x);
        nexp = normal.exp;
        nmant = normal.mant;
    }

    // Largest M. All calculations will be with the largest mantissa
    Mtype LM = std::max({x.M, res.M});
    nmant <<= (LM - x.M);

    // (1-x)/(1+x) = 1 - x
    if (precision == 0)
    {
        nexp = -nexp + (x.B + res.B - 1);
        nmant = (1 << LM) - nmant - 1;

        // normalise expects extended mantissa
        nmant += 1 << LM;
        Flexfloat norm_ans = normalise(x.s, nexp, nmant, LM, {res.E, res.M, res.B});
        res = norm_ans;
    }
    // Lagrange polinomials
    else
    {
        assert(0);
    }
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
    CLOG(trace) << std::endl;
    CLOG(trace) << "FlexFloat to_float";
    CLOG(trace) << *this;
    CLOG(trace) << "exp after conversion  = " << clib::bits(nexp);
    CLOG(trace) << "mant after conversion = " << clib::bits(nmant);
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
        if ((nmant >> -(nexp+1)) % 2 == 1)
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

    auto lhs_e = lhs.e + Flexfloat::msb(lhs_m) - lhs.B;
    auto rhs_e = rhs.e + Flexfloat::msb(rhs_m) - rhs.B;

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


void to_flexfloat(const Flexfixed &value, Flexfloat& res){
#ifdef BOOST_LOGS
    CLOG(trace) << "from_fx_to_ff";
    Flexfloat::check_ffs({res});
    CLOG(trace) << "value: " << value;
#endif

    Flexfixed::wtype msb = Flexfixed::msb(value);

    assert(res.max_exp() > msb + res.B - value.get_F());
    res.e = msb + res.B - value.get_F();

    assert(res.e < res.max_exp());

    Flexfixed::wtype delta = msb - res.M;

    if(delta >= 0){
        assert(res.max_mant() > (value.get_n() - (1 << msb)) >> delta);
        res.m = (value.get_n() - (1 << msb)) >> delta;
    }
    else {
        assert(res.max_mant() > (value.get_n() - (1 << msb)) << -delta);
        res.m = (value.get_n() - (1 << msb)) << -delta;
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
    CLOG(trace) << "============== Values before normalisation ==================";
    CLOG(trace) << "exp:  " << clib::bits(cur_exp);
    CLOG(trace) << "mant: " << clib::bits(cur_mant);
    CLOG(trace) << "=============================================================";
#endif

    if (cur_exp > 0 && cur_mant == 0)
    {
        cur_exp -= 1;
        cur_mant = 1;
    }

    auto rshift = [&cur_exp, &cur_mant](eexttype n) {
        assert(n >= 0);
        $(CLOG(trace) << "rshift on n = " << static_cast<uint64_t>(n));
        if (n > msb(cur_mant))
            cur_mant = 0;
        else
            cur_mant = cur_mant >> n;
        cur_exp = n + cur_exp;
    };
    auto lshift = [&cur_exp, &cur_mant](eexttype n) {
        assert(n >= 0);
        $(CLOG(trace) << "lshift on n = " << static_cast<uint64_t>(n));
        cur_mant = cur_mant << n;
        cur_exp = cur_exp - n;
    };

    auto ovf = [&cur_exp, &cur_mant, curM, res]() {
        $(CLOG(trace) << "overflow");
        cur_mant = max_mant(curM + 1);
        cur_exp = max_exp(res.E);
    };
    auto unf = [&cur_exp, &cur_mant, curM, res]() {
        $(CLOG(trace) << "underflow");
        cur_mant = 0;
        cur_exp = 0;
    };

    eexttype delta_m = (cur_mant > 0) ? std::abs(msb(cur_mant) - curM) : 0;
    eexttype delta_e = std::abs(cur_exp - max_exp(res.E));

    if (cur_exp <= 0)
    {
        // We must decrease mantissa, unless exponent != 0
        $(CLOG(trace) << "cur_exp <= 0");

        if (cur_mant == 0)
        {
            $(CLOG(trace) << "cur_mant = 0");
            unf();
        }
        else if (msb(cur_mant) <= curM)
        {
            $(CLOG(trace) << "msb(cur_mant) <= curM");
            rshift(-cur_exp);
        }
        else
        {
            $(CLOG(trace) << "msb(cur_mant) > curM");
            if (delta_m > delta_e)
                ovf();
            else
                rshift(std::max(-cur_exp, delta_m));
        }
    }
    else if (cur_exp > 0 && cur_exp <= max_exp(res.E))
    {
        // Exponent is normal. We must normalise mantissa
        $(CLOG(trace) << "cur_exp > 0 && cur_exp <= max_exp(res.E)");

        if (msb(cur_mant) <= curM)
        {
            $(CLOG(trace) << "msb(cur_mant) <= curM");
            lshift(std::min(cur_exp, delta_m));
        }
        else
        {
            $(CLOG(trace) << "msb(cur_mant) > curM");
            if (delta_m > delta_e)
                ovf();
            else
                rshift(delta_m);
        }
    }
    else
    {
        // Exponent is big. We must decrease it, unless exponent > max_exp
        $(CLOG(trace) << "cur_exp > max_exp(res.E)");

        if (msb(cur_mant) <= curM)
        {
            $(CLOG(trace) << "msb(cur_mant) <= curM");

            if (delta_m < delta_e)
                ovf();
            else
                lshift(std::min(delta_m, cur_exp));
        }
        else
        {
            $(CLOG(trace) << "msb(cur_mant) > curM");
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
    CLOG(trace) << "================ Values after normalisation =================";
    CLOG(trace) << "exp:  " << clib::bits(cur_exp);
    CLOG(trace) << "mant: " << clib::bits(cur_mant);
    CLOG(trace) << "=============================================================";
    CLOG(trace) << "===================== Values after zip ======================";
    CLOG(trace) << "exp:  " << clib::bits(exp);
    CLOG(trace) << "mant: " << clib::bits(mant);
    CLOG(trace) << "=============================================================";
#endif

    return Flexfloat(res.E, res.M, res.B, cur_sign, exp, mant);
}

void Flexfloat::negative(const Flexfloat &val, Flexfloat &res){
#ifdef BOOST_LOGS
    CLOG(trace) << "negative";
    Flexfloat::check_ffs({val,res});
    CLOG(trace) << "Value: " << val;
    CLOG(trace) << "Result: " << res;
#endif

    assert(val.E == res.E);
    assert(val.M == res.M);

    res.e = val.e;
    res.m = val.m;
    res.s = val.s >= 1?0:1;
    $(CLOG(trace) << "res: " << res);
}

// if e > 0  -> normalized value   -> m' = 2^M + m
// if e == 0 -> denormalized value -> m' = 2*m
Flexfloat::mtype Flexfloat::zip(eexttype exp, mexttype ext_mant, Mtype curM, Mtype reqM)
{
    if (exp > 0)
    {
        $(CLOG(trace) << "exp > 0: Value is normalized");

        assert(ext_mant >= max_mant(curM));
        ext_mant -= max_mant(curM) + 1;
    }
    else
    {
        $(CLOG(trace) << "exp == 0: Value is denormalized");

        ext_mant >>= 1;
    }

    $(CLOG(trace) << "ext_mant = " << clib::bits(ext_mant));

    if (curM >= reqM)
    {
        $(CLOG(trace) << "curM >= reqM");

        ext_mant = ext_mant >> (curM - reqM);
    }
    else
    {
        $(CLOG(trace) << "curM < reqM");

        ext_mant = ext_mant << (reqM - curM);
    }

    assert(ext_mant <= max_mant(reqM));
    return static_cast<mtype>(ext_mant);
}


void Flexfloat::abs(const Flexfloat& val, Flexfloat &res){
#ifdef BOOST_LOGS
    CLOG(trace) << "abs";
    Flexfloat::check_ffs({val,res});
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
