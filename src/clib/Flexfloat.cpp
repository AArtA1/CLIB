#include <clib/Flexfloat.hpp>

#include "clib/logs.hpp"

#include <ieee754.h>

namespace clib
{

#ifdef EN_LOGS
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
#ifdef EN_LOGS
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

Flexfloat::Flexfloat(Etype E_n, Mtype M_n, Btype B_n, mtype value) : B(B_n), E(E_n), M(M_n), s(0), e(0), m(0)
{
    m = static_cast<mtype>((value >> 0) & ((1u << M) - 1u));
    e = static_cast<etype>((value >> M) & ((1u << E) - 1u));
    s = static_cast<stype>((value >> (M + E)) & ((1u << S) - 1u));

    if (!is_valid())
    {
#ifdef EN_LOGS
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

Flexfloat::Flexfloat(const Flexfloat &hyperparams, mtype value) : B(0), E(0), M(0), s(0), e(0), m(0)
{
    B = hyperparams.get_B();
    E = hyperparams.get_E();
    M = hyperparams.get_M();

    m = static_cast<mtype>((value >> 0) & ((1u << M) - 1u));
    e = static_cast<etype>((value >> M) & ((1u << E) - 1u));
    s = static_cast<stype>((value >> (M + E)) & ((1u << S) - 1u));

    if (!is_valid())
    {
#ifdef EN_LOGS
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
    return (static_cast<etype>(1) << E) - 1;
}
Flexfloat::mtype Flexfloat::max_mant() const
{
    return (static_cast<mtype>(1) << M) - 1;
}
Flexfloat::etype Flexfloat::max_exp(Etype E)
{
    return (static_cast<etype>(1) << E) - 1;
}
Flexfloat::mtype Flexfloat::max_mant(Mtype M)
{
    return (static_cast<mtype>(1) << M) - 1;
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
#ifdef EN_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "Multiplication of two numbers";
    check_ffs({lhs, rhs, res});
    CLOG(trace) << "Left  operand: " << lhs;
    CLOG(trace) << "Right operand: " << rhs;
#endif

    // Corner cases
    if (is_zero(lhs) || is_zero(rhs))
    {
        res.e = res.m = 0;
        res.s = lhs.s ^ rhs.s;
        return;
    }

    uint8_t nsign = lhs.s ^ rhs.s;
    eexttype nexp = +static_cast<eexttype>(lhs.e) + rhs.e + res.B - lhs.B - rhs.B;

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
#ifdef EN_LOGS
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
    etype nexp = 0;
    if (lhs_e - lhs.B > rhs_e - rhs.B)
    {
        auto delta_e = lhs_e - rhs_e - lhs.B + rhs.B;
        rhs_e += delta_e;
        rhs_m >>= delta_e;

        assert(lhs_e - lhs.B + res.B >= 0);
        nexp = static_cast<etype>(lhs_e - lhs.B + res.B);
    }
    else
    {
        auto delta_e = rhs_e - lhs_e - rhs.B + lhs.B;
        lhs_e += delta_e;
        lhs_m >>= delta_e;

        assert(rhs_e - rhs.B + res.B >= 0);
        nexp = static_cast<etype>(rhs_e - rhs.B + res.B);
    }

#ifdef EN_LOGS
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
#ifdef EN_LOGS
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

    eexttype ext_exp = static_cast<eexttype>(1) - delta_N;
    ext_mant = n << delta_N;

#ifdef EN_LOGS
    CLOG(trace) << "Extended exp  = " << clib::bits(ext_exp);
    CLOG(trace) << "Extended mant = " << clib::bits(ext_mant);
#endif

    return ext_ff{ext_exp, ext_mant};
}

void Flexfloat::inv(const Flexfloat &x, Flexfloat &res)
{
    size_t precision = 0; // TODO
#ifdef EN_LOGS
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
        nexp = -nexp + (x.B + res.B - static_cast<eexttype>(1));
        nmant = static_cast<mexttype>(1 << LM) - nmant - 1;

        // normalise expects extended mantissa
        nmant += static_cast<mexttype>(1 << LM);
        Flexfloat norm_ans = normalise(x.s, nexp, nmant, LM, {res.E, res.M, res.B});
        res = norm_ans;
    }
    // Lagrange polinomials
    else
    {
        assert(0);
    }
}

Flexfloat Flexfloat::ff_from_int(Etype E, Mtype M, Btype B, int n)
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

    Mtype N = msb(static_cast<mexttype>(n));
    mexttype mant = 0;
    mexttype delta_N = static_cast<mexttype>(n) - (1 << N);
    if (N > M)
        mant = delta_N >> (N - M);
    else
        mant = delta_N << (M - N);

    // normalise expects extended mantissa
    mant += static_cast<mexttype>(1 << M);
    return normalise(s, N + B, mant, M, {E, M, B});
}

int Flexfloat::ceil() const
{
#ifdef EN_LOGS
    CLOG(trace) << "ceil";
    check_ffs({*this});
    CLOG(trace) << "ff: " << *this;
#endif

    assert(e < std::numeric_limits<Btype>::max());
    Btype eps = static_cast<Btype>(e) - B;
    if (eps < 0)
        return 0;
    else if (eps == 0)
        return s == 0 ? 1 : -1;
    else if (eps <= M)
    {
        assert(m / (1 << (M - eps)) < std::numeric_limits<int>::max());
        int m0 = static_cast<int>(m / (1 << (M - eps)));
        int ceiled = (1 << eps) + m0;
        return s > 0 ? ceiled : -ceiled;
    }
    else
    {
        size_t shift = static_cast<size_t>(eps - M);
        int ceiled = static_cast<int>(1 << shift * ((1 << M) + m));
        return s > 0 ? ceiled : -ceiled;
    }
}

// double Flexfloat::to_double() const
// {
//     return 0;
// } TODO

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
    nmant += static_cast<mexttype>(1 << M);

    // CONVERAION
    nexp = nexp - B + B_FLOAT;
    if (M_FLOAT > M)
        nmant = nmant * (1 << (M_FLOAT - M));
    else
        nmant = nmant / (1 << (M - M_FLOAT));

#ifdef EN_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "FlexFloat to_float";
    CLOG(trace) << *this;
    CLOG(trace) << "exp after conversion  = " << clib::bits(nexp);
    CLOG(trace) << "mant after conversion = " << clib::bits(nmant);
#endif

    Flexfloat ans = normalise(s, nexp, nmant, M_FLOAT, {E_FLOAT, M_FLOAT, B_FLOAT});

    ieee754_float fc;
    assert(ans.e <= static_cast<etype>(1 << E_FLOAT));
    assert(ans.s <= 1);
    assert(ans.m <= static_cast<mtype>(1 << M_FLOAT));

#pragma GCC diagnostic ignored "-Wconversion"
    fc.ieee.negative = ans.s;
    fc.ieee.exponent = ans.e;
    fc.ieee.mantissa = ans.m;
#pragma GCC diagnostic warning "-Wconversion"

    return fc.f;
}
#pragma GCC diagnostic warning "-Wstrict-overflow"

Flexfloat Flexfloat::from_float(Etype E, Mtype M, Btype B, float flt)
{
    ieee754_float fc;
    fc.f = flt;

#pragma GCC diagnostic ignored "-Wconversion"
    stype nsign = fc.ieee.negative;
    eexttype nexp = fc.ieee.exponent;
    mexttype nmant = fc.ieee.mantissa;
#pragma GCC diagnostic warning "-Wconversion"
    // get extended mantissa
    nmant += static_cast<mexttype>(1 << M_FLOAT);

    // CONVERAION
    nexp = nexp - B_FLOAT + B;
    if (M_FLOAT > M)
        nmant = nmant / (1 << (M_FLOAT - M));
    else
        nmant = nmant * (1 << (M - M_FLOAT));

#ifdef EN_LOGS
    CLOG(trace) << std::endl;
    CLOG(trace) << "FlexFloat from_float = " << flt;
    CLOG(trace) << "exp after conversion  = " << clib::bits(nexp);
    CLOG(trace) << "mant after conversion = " << clib::bits(nmant);
#endif

    return normalise(nsign, nexp, nmant, M, {E, M, B});
}

Flexfloat Flexfloat::from_float(const Flexfloat &hyperparams, float flt)
{
    return from_float(hyperparams.E, hyperparams.M, hyperparams.B, flt);
}


void Flexfloat::from_float(float flt, const Flexfloat &in, Flexfloat &out)
{
    out = from_float(in.E, in.M, in.B, flt);
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
#ifdef EN_LOGS
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

    auto lhs_e = static_cast<Flexfloat::eexttype>(lhs.e) + Flexfloat::msb(lhs_m) - lhs.B;
    auto rhs_e = static_cast<Flexfloat::eexttype>(rhs.e) + Flexfloat::msb(rhs_m) - rhs.B;

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
#ifdef EN_LOGS
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
#ifdef EN_LOGS
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
#ifdef EN_LOGS
    CLOG(trace) << "clip";
    Flexfloat::check_ffs({a, x, b, out});
    CLOG(trace) << "a: " << a;
    CLOG(trace) << "x: " << x;
    CLOG(trace) << "b: " << b;
#endif

    min(x, b, out);
    max(a, out, out);
}

//
// See
// gitlab.inviewlab.com/synthesizer/documents/-/blob/master/out/flexfloat_normalize.pdf
//
Flexfloat Flexfloat::normalise(stype cur_sign, eexttype cur_exp, mexttype cur_mant, Mtype curM, hyper_params res)
{
#ifdef EN_LOGS
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

    eexttype delta_m = (cur_mant > 0) ? abs<eexttype>(msb(cur_mant) - curM) : 0;
    eexttype delta_e = abs<eexttype>(cur_exp - max_exp(res.E));

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
    assert(cur_exp >= 0);
    assert(cur_exp <= max_exp(res.E));
    assert(msb(cur_mant) <= sizeof(mtype) * 8);

    mtype mant = zip(cur_exp, cur_mant, curM, res.M);
    etype exp = static_cast<etype>(cur_exp);

#ifdef EN_LOGS
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
        return (2 * static_cast<mexttype>(mant));
    else
        return ((1 << M) + static_cast<mexttype>(mant));
}
Flexfloat::mexttype Flexfloat::unzip(const Flexfloat &ff)
{
    if (ff.e == 0)
        return (2 * static_cast<mexttype>(ff.m));
    else
        return ((1 << ff.M) + static_cast<mexttype>(ff.m));
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
    if (e > static_cast<etype>((1 << E) - 1))
    {
        $(CLOG(error) << "e > static_cast<etype>((1 << E) - 1)");
        return false;
    }
    if (m > static_cast<mtype>((1 << M) - 1))
    {
        $(CLOG(error) << "m > static_cast<mtype>((1 << M) - 1)");
        return false;
    }

    return true;
}

} // namespace clib
