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

    CLOG(trace) << "Object successfully created";
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

    CLOG(trace) << "Object successfully created";
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


void Flexfloat::mult(
    const Flexfloat &left, const Flexfloat &right, Flexfloat &res)
{
    CLOG(trace) << "Multiplication of two numbers";

    if (!left.is_valid())
    {
        CLOG(error) << "Left operand is invalid. Can not multiplicate";
        throw std::runtime_error{"Invalid operand"};
    }
    if (!right.is_valid())
    {
        CLOG(error) << "Right operand is invalid. Can not multiplicate";
        throw std::runtime_error{"Invalid operand"};
    }
    if (!res.is_valid())
    {
        CLOG(error) << "Result operand is invalid. Can not multiplicate";
        throw std::runtime_error{"Invalid operand"};
    }

    CLOG(trace) << "Left  operand:" << std::endl
                                << left;
    CLOG(trace) << "Right operand:" << std::endl
                                << right;

    uint8_t nsign = left.s ^ right.s;

    // Corner cases
    if (is_zero(left) || is_zero(right))
    {
        res.e = res.m = 0;
        res.s = nsign;
        return;
    }

    eexttype nexp = +static_cast<int128_t>(left.e) 
                    +right.e 
                    +res.B 
                    -left.B 
                    -right.B;

    // Largest M. All calculations will be with the largest mantissa
    Mtype LM = std::max({left.M, right.M, res.M});

    // Casting all mantissa's to LM
    mtype left_m  = left.m  << (LM - left.M);
    mtype right_m = right.m << (LM - right.M);

    // if e > 0  -> normalized value   -> m' = 2^M + m
    // if e == 0 -> denormalized value -> m' = 2*m
    //
    // m'_res = m'_left  * m'_right >> LM
    mexttype nmant = 1;

    if (left.e == 0)
        nmant *= ( 2 * static_cast<mexttype>(left_m) );
    else
        nmant *= ( (1 << LM) + static_cast<mexttype>(left_m) );

    if (right.e == 0)
        nmant *= ( 2 * static_cast<mexttype>(right_m) );
    else
        nmant *= ( (1 << LM) + static_cast<mexttype>(right_m) );

    Flexfloat norm_ans = normalise(nsign, nexp - LM, nmant, res.E, LM, res.B);
    res = norm_ans;

    CLOG(trace) << "Result:" << std::endl << res;
    return; 
}

std::ostream& operator<<(std::ostream &oss, const Flexfloat &num)
{
    oss << "M: " << +num.M << std::endl;
    oss << "E: " << +num.E << std::endl;
    oss << "B: " << +num.B << std::endl;

    oss << "Bits: " << num.bits() << std::endl << std::endl;

    return oss;
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
    llu_t cur_exp_printable  = *reinterpret_cast<llu_t *>(&cur_exp);
    llu_t cur_mant_printable = *reinterpret_cast<llu_t *>(&cur_mant);
    CLOG(trace) << "==================== Values before normalisation =====================";
    CLOG(trace) << "exp:  " << std::bitset<sizeof(llu_t)*8>(cur_exp_printable);
    CLOG(trace) << "      " << cur_exp_printable;
    CLOG(trace) << "mant: " << std::bitset<sizeof(llu_t)*8>(cur_mant_printable);
    CLOG(trace) << "      " << cur_mant_printable;
    CLOG(trace) << "======================================================================";

    if (cur_exp > 0 && cur_mant == 0)
    {
        cur_exp -= 1;
        cur_mant = 1;
    }

    auto rshift = [&cur_exp, &cur_mant](int128_t n) 
    { 
        CLOG(trace) << "rshift on n = " << static_cast<uint64_t>(n);
        cur_mant = cur_mant >> n;
        cur_exp = n + cur_exp;
    };
    auto lshift = [&cur_exp, &cur_mant](int128_t n) 
    { 
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
        CLOG(trace) << "cur_exp < 0";

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
                std::min(delta_m, cur_exp);
        }
        else
        {
            CLOG(trace) << "msb(cur_mant) > M";
            ovf();
        }
    }

    cur_exp_printable  = *reinterpret_cast<llu_t *>(&cur_exp);
    cur_mant_printable = *reinterpret_cast<llu_t *>(&cur_mant);
    CLOG(trace) << "=================== Values after pre-normalisation ===================";
    CLOG(trace) << "exp:  " << std::bitset<sizeof(llu_t)*8>(cur_exp_printable);
    CLOG(trace) << "      " << cur_exp_printable;
    CLOG(trace) << "mant: " << std::bitset<sizeof(llu_t)*8>(cur_mant_printable);
    CLOG(trace) << "      " << cur_mant_printable;
    CLOG(trace) << "======================================================================";

    assert(cur_exp >= 0);
    if (cur_exp > 0)
    {
        CLOG(trace) << "cur_exp > 0: Value is normalized";

        assert(cur_mant >= max_mant(M));
        cur_mant -= static_cast<uint128_t>(max_mant(M) + 1);
    }
    else
    {
        CLOG(trace) << "cur_exp == 0: Value is denormalized";

        cur_mant >>= 1;
    }

    cur_exp_printable  = *reinterpret_cast<llu_t *>(&cur_exp);
    cur_mant_printable = *reinterpret_cast<llu_t *>(&cur_mant);
    CLOG(trace) << "===================== Values after normalisation ====================";
    CLOG(trace) << "exp:  " << std::bitset<sizeof(llu_t)*8>(cur_exp_printable);
    CLOG(trace) << "      " << cur_exp_printable;
    CLOG(trace) << "mant: " << std::bitset<sizeof(llu_t)*8>(cur_mant_printable);
    CLOG(trace) << "      " << cur_mant_printable;
    CLOG(trace) << "======================================================================";

    assert(E <= 64);
    assert(M <= 64);
    assert(cur_exp >= 0);
    assert(msb(static_cast<uint128_t>(cur_exp)) <= 63);
    assert(msb(cur_mant) <= 63);

    return Flexfloat(
        E, M, B, 
        cur_sign, static_cast<etype>(cur_exp), static_cast<mtype>(cur_mant)
    );
}

bool Flexfloat::is_valid() const
{
    if (!(M <= sizeof(m)*8))
    {
        CLOG(error) << "!(M <= sizeof(m)*8)";
        return false;
    }
    if (!(E <= sizeof(e)*8))
    {
        CLOG(error) << "!(E <= sizeof(e)*8)";
        return false;
    }

    if (!(s <= 1))
    {
        CLOG(error) << "!(s <= 1)";
        return false;
    }
    if (!(e <= static_cast<etype>((1 << E) - 1)))
    {
        CLOG(error) << "!(e <= static_cast<etype>((1 << E) - 1))";
        return false;
    }
    if (!(m <= static_cast<mtype>((1 << M) - 1)))
    {
        CLOG(error) << "!(m <= static_cast<mtype>((1 << M) - 1))";
        return false;
    }

    CLOG(trace) << "Object is valid";
    return true;

}

} //namespace clib 
