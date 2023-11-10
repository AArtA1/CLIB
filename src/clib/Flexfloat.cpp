#include <clib/Flexfloat.hpp>
#include "clib/logs.hpp"

namespace clib {

Flexfloat::Flexfloat(Etype E_n, Mtype M_n, Btype B_n, stype s_n, etype e_n, mtype m_n) :
    B(B_n),
    E(E_n),
    M(M_n),
    s(s_n),
    e(e_n),
    m(m_n)
{
    if (!is_valid())
    {
        CLOG(error) << "Can not create object. Invalid parameters";
        throw std::runtime_error{"Can not create object. Invalid parameters"};
    }
}

Flexfloat::Flexfloat(Etype E_n, Mtype M_n, Btype B_n, mtype value) :
    B(B_n),
    E(E_n),
    M(M_n),
    s(0),
    e(0),
    m(0)
{
    m = static_cast<mtype>((value >> 0)     & ((1u << M) - 1u));
    e = static_cast<etype>((value >> M)     & ((1u << E) - 1u));
    s = static_cast<stype>((value >> (M+E)) & ((1u << S) - 1u));   

    if (!is_valid())
    {
        CLOG(error) << "Can not create object. Invalid parameters";
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

Flexfloat::Mtype Flexfloat::msb(uint128_t val)
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

bool Flexfloat::is_zero(const Flexfloat& val)
{
    if (val.m == 0 && val.e == 0) return true;
    else return false;
}

Flexfloat& Flexfloat::operator=(const Flexfloat& other)
{
    assert(B == other.B);
    assert(E == other.E);

    if (other.M >= M)
    {
        m = other.m >> (other.M - M); 
    }
    if (other.M < M)
    {
        m = other.m << (M - other.M);
    }

    e = other.e;
    s = other.s;

    return *this;
}

void Flexfloat::check_ffs(std::initializer_list<Flexfloat> list)
{
    for (auto& elem : list)
    {
        if (!elem.is_valid())
        {
            CLOG(error) << "Operand is invalid. Can not multiplicate";
            throw std::runtime_error{"Invalid operand"};
        }
    }
}

void Flexfloat::mult(
    const Flexfloat &left, const Flexfloat &right, Flexfloat &res)
{
#ifndef NDEBUG
    CLOG(trace) << "Multiplication of two numbers";
    check_ffs({left, right, res});
    CLOG(trace) << "Left  operand: " << left;
    CLOG(trace) << "Right operand: " << right;
#endif

    // Corner cases
    if (is_zero(left) || is_zero(right))
    {
        res.e = res.m = 0;
        res.s = left.s ^ right.s;
        return;
    }
    
    mexttype left_m  = unzip(left);
    mexttype right_m = unzip(right);

    uint8_t nsign = left.s ^ right.s;
    eexttype nexp = +static_cast<int128_t>(left.e) 
                    +right.e 
                    +res.B 
                    -left.B 
                    -right.B;

    // Largest M. All calculations will be with the largest mantissa
    Mtype LM = std::max({left.M, right.M, res.M});

    // Casting all mantissa's to LM
    left_m  <<= (LM - left.M);
    right_m <<= (LM - right.M);

    mexttype nmant = left_m * right_m;

    Flexfloat norm_ans = normalise(nsign, nexp - LM, nmant, res.E, LM, res.B);
    res = norm_ans;

#ifndef NDEBUG
    CLOG(trace) << "Result: " << res;
#endif
    return; 
}

std::ostream& operator<<(std::ostream &oss, const Flexfloat &num)
{
    oss << "(E, M, B) = (" << +num.E << ", " << +num.M << ", " << +num.B << ")";
    oss << "  val = " << num.bits();

    return oss;
}

void Flexfloat::sum(
    const Flexfloat &left, const Flexfloat &right, Flexfloat &res)
{
#ifndef NDEBUG
    CLOG(trace) << "Sum of two numbers";
    check_ffs({left, right, res});
    CLOG(trace) << "Left  operand: " << left;
    CLOG(trace) << "Right operand: " << right;
#endif

    mexttype left_m  = unzip(left);
    mexttype right_m = unzip(right);

    // Largest M. All calculations will be with the largest mantissa
    Mtype LM = std::max({left.M, right.M, res.M});

    // Casting all mantissa's to LM
    left_m  <<= (LM - left.M);
    right_m <<= (LM - right.M);
    eexttype right_e = right.e;
    eexttype left_e  = left.e;

    // Casting inputs to maximum exponent
    etype nexp = 0;
    if (left_e - left.B > right_e - right.B)
    {
        auto delta_e = left_e - right_e - left.B + right.B;
        right_e += delta_e;
        right_m >>= delta_e;

        assert(left_e - left.B + res.B >= 0);
        nexp = static_cast<etype>(left_e - left.B + res.B);
    }
    else 
    {
        auto delta_e = right_e - left_e - right.B + left.B;
        left_e += delta_e;
        left_m >>= delta_e;

        assert(right_e - right.B + res.B >= 0);
        nexp = static_cast<etype>(right_e - right.B + res.B);
    }

#ifndef NDEBUG
    CLOG(trace) << "=================== Values after casting ====================";
    CLOG(trace) << "left_e:  " << clib::bits(left_e);
    CLOG(trace) << "left_m:  " << clib::bits(left_m);
    CLOG(trace) << "right_e: " << clib::bits(right_e);
    CLOG(trace) << "right_m: " << clib::bits(right_m);
    CLOG(trace) << "=============================================================";
#endif

    mexttype nmant = 0;
    stype nsign = 0;

    if (left.s == right.s)
    {
        nsign = left.s;
        nmant = left_m + right_m;
    }
    else if (left_m >= right_m)
    {
        nsign = left.s;
        nmant = left_m - right_m; 
    }
    else
    {
        nsign = right.s;
        nmant = right_m - left_m; 
    }

    Flexfloat norm_ans = normalise(nsign, nexp, nmant, res.E, LM, res.B);
    res = norm_ans;

    CLOG(trace) << "Result: " << res;
    return; 
}

Flexfloat::ext_ff Flexfloat::get_normalized(const Flexfloat &denorm)
{
#ifndef NDEBUG
    CLOG(trace) << "Getting normal value of denormal";
    check_ffs({denorm});
    CLOG(trace) << "Value " << denorm;

    assert(denorm.e == 0);
#endif

    if (denorm.m == 0)
        return ext_ff {0, 0};

    auto N = msb(denorm.m);
    auto n = denorm.m - (1 << msb(denorm.m));
    auto delta_N = denorm.M - N;

    auto ext_exp = static_cast<eexttype>(1) - delta_N;
    auto ext_mant = n << delta_N;

#ifndef NDEBUG
    CLOG(trace) << "Extended exp  = " << static_cast<int64_t>(ext_exp);
    CLOG(trace) << "Extended mant = " << clib::bits(ext_mant);
#endif

    return ext_ff {ext_exp, ext_mant};
}

void Flexfloat::inv(const Flexfloat &x, Flexfloat &res, size_t precision)
{
#ifndef NDEBUG
    CLOG(trace) << "Inv: 1/x";
    check_ffs({x, res});
    CLOG(trace) << "x: " << x;
#endif

    if (x.m == 0 && x.e == 0) 
        throw std::runtime_error("division by zero");

    eexttype nexp = x.e;
    mexttype nmant = x.m;
    if (x.e == 0)
    {
        ext_ff normal = get_normalized(x);
        nexp  = normal.exp;
        nmant = normal.mant;
    }

    // (1-x)/(1+x) = 1 - x
    if (precision == 0)
    {
        nexp  = -nexp + (x.B + res.B - static_cast<eexttype>(1));
        nmant = static_cast<mexttype>(1 << x.M) - x.m;
        
        // normalise expects extended mantissa
        nmant += static_cast<mexttype>(1 << x.M);
        Flexfloat norm_ans = normalise(x.s, nexp, nmant, res.E, res.M, res.B);
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
#ifndef NDEBUG
    CLOG(trace) << "FlexFloat from int = " << n;
#endif

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

    Mtype N = msb(static_cast<uint128_t>(n));
    mexttype mant = 0;
    mexttype delta_N = static_cast<mexttype>(n) - (1 << N);
    if (N > M)
        mant = delta_N >> (N-M);
    else
        mant = delta_N << (M-N);

    // normalise expects extended mantissa
    mant += static_cast<mexttype>(1 << M);
    return normalise(s, N + B, mant, E, M, B);
}

int Flexfloat::ceil() const
{
#ifndef NDEBUG
    CLOG(trace) << "ceil";
    check_ffs({*this});
    CLOG(trace) << "ff: " << *this;

    // assert(e < std::numeric_limits<Btype>::max); TODO
#endif

    Btype eps = static_cast<Btype>(e) - B;
    if (eps < 0)
        return 0;
    else if (eps == 0)
        return s == 0 ? 1 : -1;
    else if (eps <= M)
    {
        // TODO asserts
        int m0 = static_cast<int>(m / (1 << (M-eps)));
        int ceiled = (1 << eps) + m0;
        return s > 0 ? ceiled : -ceiled;
    }
    else
    {
        // TODO asserts
        size_t shift = static_cast<size_t>(eps-M);
        int ceiled = static_cast<int>(1 << shift * ((1 << M) + m));
        return s > 0 ? ceiled : -ceiled;
    }
}

std::string Flexfloat::bits() const
{
    const size_t s_size = sizeof(s) * 8;
    const size_t e_size = sizeof(e) * 8;
    const size_t m_size = sizeof(m) * 8;

    std::string sign_s = std::bitset<s_size>(s).to_string();
    std::string exp_s  = std::bitset<e_size>(e).to_string();
    std::string mant_s = std::bitset<m_size>(m).to_string();

    sign_s = sign_s.substr(s_size - S, std::string::npos);
    exp_s  = exp_s.substr (e_size - E, std::string::npos);
    mant_s = mant_s.substr(m_size - M, std::string::npos);

    return sign_s + "|" + exp_s + "|" + mant_s;
}

//
// See gitlab.inviewlab.com/synthesizer/documents/-/blob/master/out/flexfloat_normalize.pdf
//
Flexfloat Flexfloat::normalise(
    stype cur_sign, eexttype cur_exp, mexttype cur_mant, Etype E, Mtype M, Btype B
)
{
#ifndef NDEBUG
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

    auto rshift = [&cur_exp, &cur_mant](int128_t n) 
    {
        assert(n >= 0);
        CLOG(trace) << "rshift on n = " << static_cast<uint64_t>(n);
        cur_mant = cur_mant >> n;
        cur_exp = n + cur_exp;
    };
    auto lshift = [&cur_exp, &cur_mant](int128_t n) 
    {
        assert(n >= 0);
        CLOG(trace) << "lshift on n = " << static_cast<uint64_t>(n);
        cur_mant = cur_mant << n;
        cur_exp = cur_exp - n;
    };

    auto ovf = [&cur_exp, &cur_mant, M, E]() 
    { 
        CLOG(trace) << "overflow";
        cur_mant = max_mant(M+1);
        cur_exp = max_exp(E);
    };
    auto unf = [&cur_exp, &cur_mant, M, E]() 
    { 
        CLOG(trace) << "underflow";
        cur_mant = 0;
        cur_exp = 0;
    };

    int128_t delta_m = (cur_mant > 0) ? abs<int128_t>(msb(cur_mant) - M) : 0;
    int128_t delta_e = abs<int128_t>(cur_exp - max_exp(E));

    if (cur_exp <= 0)
    {
        // We must decrease mantissa, unless exponent != 0
        CLOG(trace) << "cur_exp <= 0";

        if (cur_mant == 0)
        {
            CLOG(trace) << "cur_mant = 0";
            unf();
        }
        else if (msb(cur_mant) <= M)
        {
            CLOG(trace) << "msb(cur_mant) <= M";
            rshift(-cur_exp);
        }
        else
        {
            CLOG(trace) << "msb(cur_mant) > M";
            if (delta_m > delta_e)
                ovf();
            else
                rshift(std::max(-cur_exp, delta_m));
        }
    }
    else if (cur_exp > 0 && cur_exp <= max_exp(E))
    { 
        // Exponent is normal. We must normalise mantissa
        CLOG(trace) << "cur_exp > 0 && cur_exp <= max_exp(E)";

        if (msb(cur_mant) <= M)
        {
            CLOG(trace) << "msb(cur_mant) <= M";
            lshift(std::min(cur_exp, delta_m));
        }
        else
        {
            CLOG(trace) << "msb(cur_mant) > M";
            if (delta_m > delta_e)
                ovf();
            else
                rshift(delta_m);
        }
    }
    else
    { 
        // Exponent is big. We must decrease it, unless exponent > max_exp
        CLOG(trace) << "cur_exp > max_exp(E)";

        if (msb(cur_mant) <= M)
        {
            CLOG(trace) << "msb(cur_mant) <= M";

            if (delta_m < delta_e)
                ovf();
            else
                lshift(std::min(delta_m, cur_exp));
        }
        else
        {
            CLOG(trace) << "msb(cur_mant) > M";
            ovf();
        }
    } 
    
    assert(E < sizeof(etype)*8);
    assert(M < sizeof(mtype)*8);
    assert(cur_exp >= 0);
    assert(cur_exp <= max_exp(E));
    assert(msb(cur_mant) <= sizeof(mtype)*8);

    mtype mant = zip(cur_exp, cur_mant, M);
    etype exp = static_cast<etype>(cur_exp);

#ifndef NDEBUG
    CLOG(trace) << "================ Values after normalisation =================";
    CLOG(trace) << "exp:  " << clib::bits(cur_exp);
    CLOG(trace) << "mant: " << clib::bits(cur_mant);
    CLOG(trace) << "=============================================================";
    CLOG(trace) << "===================== Values after zip ======================";
    CLOG(trace) << "exp:  " << clib::bits(exp);
    CLOG(trace) << "mant: " << clib::bits(mant);
    CLOG(trace) << "=============================================================";
#endif

    return Flexfloat(E, M, B, cur_sign, exp, mant);
}

// if e > 0  -> normalized value   -> m' = 2^M + m
// if e == 0 -> denormalized value -> m' = 2*m
Flexfloat::mtype Flexfloat::zip(eexttype exp, mexttype ext_mant, Mtype M)
{
    if (exp > 0)
    {
        CLOG(trace) << "exp > 0: Value is normalized";

        assert(ext_mant >= max_mant(M));
        ext_mant -= max_mant(M) + 1;
    }
    else
    {
        CLOG(trace) << "exp == 0: Value is denormalized";

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
        return ( 2 * static_cast<mexttype>(mant) );
    else
        return ( (1 << M) + static_cast<mexttype>(mant) );
}
Flexfloat::mexttype Flexfloat::unzip(const Flexfloat& ff)
{
    if (ff.e == 0)
        return ( 2 * static_cast<mexttype>(ff.m) );
    else
        return ( (1 << ff.M) + static_cast<mexttype>(ff.m) );
}

 
bool Flexfloat::is_valid() const
{
    if (M > sizeof(m)*8)
    {
        CLOG(error) << "M > sizeof(m)*8";
        return false;
    }
    if (E > sizeof(e)*8)
    {
        CLOG(error) << "E > sizeof(e)*8";
        return false;
    }
    if (s > 1)
    {
        CLOG(error) << "s > 1";
        return false;
    }
    if (e > static_cast<etype>((1 << E) - 1))
    {
        CLOG(error) << "e > static_cast<etype>((1 << E) - 1)";
        return false;
    }
    if (m > static_cast<mtype>((1 << M) - 1))
    {
        CLOG(error) << "m > static_cast<mtype>((1 << M) - 1)";
        return false;
    }

    return true;

}

} //namespace clib 
