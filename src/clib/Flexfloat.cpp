#include <clib/Flexfloat.hpp>
#include "clib/logs.hpp"

namespace clib {

Flexfloat::Flexfloat(uint8_t E_n, uint8_t M_n, int B_n, uint8_t s_n, uint64_t e_n, uint64_t m_n) : 
    B(B_n),
    E(E_n),
    M(M_n),
    s(s_n),
    e(e_n),
    m(m_n)
{
    if (!is_valid())
    {
        LOG(error) << "Can not create object. Invalid parameters";
        throw std::string{"Can not create object. Invalid parameters"};
    }

    LOG(trace) << "Object successfully created";
}

inline uint64_t Flexfloat::max_exp() const
{
    return (static_cast<uint64_t>(1) << E) - 1;
}
inline uint64_t Flexfloat::max_mant() const
{
    return (static_cast<uint64_t>(1) << M) - 1;
}
inline uint64_t Flexfloat::max_exp(uint8_t E)
{
    return (static_cast<uint64_t>(1) << E) - 1;
}
inline uint64_t Flexfloat::max_mant(uint8_t M)
{
    return (static_cast<uint64_t>(1) << M) - 1;
}

uint8_t Flexfloat::msb(uint128_t val)
{
    // val must be > 0!
    if (val == 0)
        throw std::string{"Can find msb. Input must be > 0"};
    
    uint8_t msb = 0;
    while (val > 0)
    {
        val >>= 1;
        msb++;
    }
    
    return msb - 1;
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
    LOG(trace) << "Multiplication of two numbers";

    if (!left.is_valid())
    {
        LOG(error) << "Left operand is invalid. Can not multiplicate";
        throw std::string{"Invalid operand"};
    }
    if (!right.is_valid())
    {
        LOG(error) << "Right operand is invalid. Can not multiplicate";
        throw std::string{"Invalid operand"};
    }
    if (!res.is_valid())
    {
        LOG(error) << "Result operand is invalid. Can not multiplicate";
        throw std::string{"Invalid operand"};
    }

    LOG(trace) << "Left  operand:" << std::endl
                                << left;
    LOG(trace) << "Right operand:" << std::endl
                                << right;

    uint8_t nsign = left.s ^ right.s;

    int128_t nexp = +static_cast<int128_t>(left.e) 
                    +right.e 
                    +res.B 
                    -left.B 
                    -right.B;

    // Largest M. All calculations will be with the largest mantissa
    uint8_t LM = std::max({left.M, right.M, res.M});

    // Casting all mantissa's to LM
    uint64_t left_m  = left.m  << (LM - left.M);
    uint64_t right_m = right.m << (LM - right.M);
    //int64_t res_m   = res.m   << (LM - res.M);

    // if e > 0  -> normalized value   -> m' = 2^M + m
    // if e == 0 -> denormalized value -> m' = 2*m
    //
    // m'_res = m'_left  * m'_right >> LM
    //
    // Sorting out all variants of normalized/denormalized operand
    uint128_t nmant = static_cast<uint128_t>(left_m) * static_cast<uint128_t>(right_m);

    if (left.e == 0)
    {
        // 2 * left_m * 2 * right_m
        //
        // nmant = left_m * right_m >> (LM-2)
        if (right.e == 0)
        {
            nmant = nmant >> (LM - 2);
        }
        // 2 * left_m * (2^LM + right_m) = 2^(LM+1) * left_m + 2 * left_m * right_m;
        //
        // nmant = ( 2^(LM+1)*left_m + 2*left_m*right_m ) >> LM
        else
        {
            nmant = 2 * left_m + (nmant >> (LM - 1));
        }
    }
    else
    {
        // Same as left.e == 0 && right.e != 0
        if (right.e == 0)
        {
            nmant = 2 * right_m + (nmant >> (LM - 1));
        }
        // (2^LM + left_m) * (2^LM + right_m) =
        // 2^(2*LM) + 2^LM * (left_m + right_m) + left_m * right_m
        //
        // nmant = ( 2^(2*LM) + 2^LM * (left_m + right_m) + left_m * right_m ) >> LM
        else
        {
            nmant = (1 << LM) + left_m + right_m + ((left_m * right_m) >> LM);
        }
    }

    Flexfloat norm_ans = normalise(nsign, nexp, nmant, res.E, LM, res.B);
    res = norm_ans;

    LOG(trace) << "Result:" << std::endl << res;

    return; 
}

std::ostream& operator<<(std::ostream &oss, const Flexfloat &num)
{
    const size_t s_size = sizeof(num.s) * 8;
    const size_t e_size = sizeof(num.e) * 8;
    const size_t m_size = sizeof(num.m) * 8;

    oss << "M: " << +num.M << std::endl;
    oss << "E: " << +num.E << std::endl;
    oss << "B: " << +num.B << std::endl;

    std::string sign_s = std::bitset<s_size>(num.s).to_string();
    std::string exp_s  = std::bitset<e_size>(num.e).to_string();
    std::string mant_s = std::bitset<m_size>(num.m).to_string();

    sign_s = sign_s.substr(s_size - num.S, std::string::npos);
    exp_s  = exp_s.substr (e_size - num.E, std::string::npos);
    mant_s = mant_s.substr(m_size - num.M, std::string::npos);

    oss << "Sign: " << sign_s << std::endl;
    oss << "Exp:  " << exp_s  << std::endl;
    oss << "Mant: " << mant_s << std::endl << std::endl;

    return oss;
}

// Fitting values in correct range
//
// Input: -inf <  cur_exp  < +inf
//           0 <= cur_mant < +inf
//
// Desired output: Flexfloat with
//           0 <= cur_exp  < 2^E
//           0 <= cur_mant < 2^M
//
Flexfloat Flexfloat::normalise(
    uint8_t cur_sign, int128_t cur_exp, uint128_t cur_mant, uint8_t E, uint8_t M, int B
)
{
    // for logs
    llu_t cur_exp_printable  = *reinterpret_cast<llu_t *>(&cur_exp);
    llu_t cur_mant_printable = *reinterpret_cast<llu_t *>(&cur_mant);
    LOG(trace) << "==================== Values before normalisation =====================";
    LOG(trace) << "exp:  " << std::bitset<sizeof(llu_t)*8>(cur_exp_printable);
    LOG(trace) << "      " << cur_exp_printable;
    LOG(trace) << "mant: " << std::bitset<sizeof(llu_t)*8>(cur_mant_printable);
    LOG(trace) << "      " << cur_mant_printable;
    LOG(trace) << "======================================================================";
    

    if (cur_exp > 0 && cur_mant == 0)
    {
        cur_exp -= 1;
        cur_mant = 1;
    }
    int128_t delta_m = (cur_mant > 0) ? abs<int128_t>(msb(cur_mant) - M) : 0;
    int128_t delta_e = abs<int128_t>(cur_exp - max_exp(E));

    if (cur_exp <= 0)
    {
        // We must decrease mantissa, unless exponent != 0
        LOG(trace) << "cur_exp < 0";

        if (cur_mant == 0)
        {
            LOG(trace) << "cur_mant = 0";
            LOG(trace) << "undeflow";
            cur_exp = 0;
            cur_mant = 0;
        }
        else if (msb(cur_mant) <= M)
        {
            LOG(trace) << "msb(cur_mant) <= M";
            LOG(trace) << "rshift";
            uint128_t rshift = static_cast<uint128_t>(-cur_exp);
        
            cur_mant = cur_mant >> rshift;
            cur_exp = 0;
        }
        else
        {
            LOG(trace) << "msb(cur_mant) > M";
            if (delta_m > delta_e)
            {
                LOG(trace) << "overflow";
                cur_mant = max_mant(M+1);
                cur_exp = max_exp(E);
            }
            else
            {
                LOG(trace) << "rshift";
                int128_t rshift = std::max(-cur_exp, delta_m);

                cur_mant = cur_mant >> rshift;
                cur_exp = cur_exp + static_cast<int128_t>(rshift);   
            }
        }
    }
    else if (cur_exp > 0 && cur_exp <= max_exp(E))
    { 
        // Exponent is normal. We must normalise mantissa
        LOG(trace) << "cur_exp > 0 && cur_exp <= max_exp(E)";

        if (msb(cur_mant) <= M)
        {
            LOG(trace) << "msb(cur_mant) <= M";
            LOG(trace) << "lshift";

            int128_t lshift = std::min(cur_exp, delta_m);
        
            cur_mant = cur_mant << lshift;
            cur_exp = cur_exp - static_cast<int128_t>(lshift);
        }
        else
        {
            LOG(trace) << "msb(cur_mant) > M";

            if (delta_m > delta_e)
            {
                LOG(trace) << "overflow";
                cur_mant = max_mant(M+1);
                cur_exp = max_exp(E);
            }
            else
            {
                LOG(trace) << "rshift";
                int128_t rshift = delta_m ;

                cur_mant = cur_mant >> rshift;
                cur_exp = cur_exp + static_cast<int128_t>(rshift);   
            }
        }
    }
    else
    { 
        // Exponent is big. We must decrease it, unless exponent > max_exp
        LOG(trace) << "cur_exp > max_exp(E)";

        if (msb(cur_mant) <= M)
        {
            LOG(trace) << "msb(cur_mant) <= M";

            if (delta_m < delta_e)
            {
                LOG(trace) << "overflow";
                cur_mant = max_mant(M+1);
                cur_exp = max_exp(E);
            }
            else
            {
                LOG(trace) << "lshift";
                int128_t lshift = std::min(delta_m, cur_exp);

                cur_mant = cur_mant << lshift;
                cur_exp = cur_exp - static_cast<int128_t>(lshift);   
            }
        }
        else
        {
            LOG(trace) << "msb(cur_mant) > M";
            LOG(trace) << "overflow";

            cur_mant = max_mant(M+1);
            cur_exp = max_exp(E);
        }
    }

    cur_exp_printable  = *reinterpret_cast<llu_t *>(&cur_exp);
    cur_mant_printable = *reinterpret_cast<llu_t *>(&cur_mant);
    LOG(trace) << "=================== Values after pre-normalisation ===================";
    LOG(trace) << "exp:  " << std::bitset<sizeof(llu_t)*8>(cur_exp_printable);
    LOG(trace) << "      " << cur_exp_printable;
    LOG(trace) << "mant: " << std::bitset<sizeof(llu_t)*8>(cur_mant_printable);
    LOG(trace) << "      " << cur_mant_printable;
    LOG(trace) << "======================================================================";

    assert(cur_exp >= 0);
    if (cur_exp > 0)
    {
        LOG(trace) << "cur_exp > 0: Value is normalized";

        assert(cur_mant >= max_mant(M));
        cur_mant -= static_cast<uint128_t>(max_mant(M) + 1);
    }
    else
    {
        LOG(trace) << "cur_exp > 0: Value is denormalized";

        cur_mant >>= 1;
    }

    cur_exp_printable  = *reinterpret_cast<llu_t *>(&cur_exp);
    cur_mant_printable = *reinterpret_cast<llu_t *>(&cur_mant);
    LOG(trace) << "===================== Values after normalisation ====================";
    LOG(trace) << "exp:  " << std::bitset<sizeof(llu_t)*8>(cur_exp_printable);
    LOG(trace) << "      " << cur_exp_printable;
    LOG(trace) << "mant: " << std::bitset<sizeof(llu_t)*8>(cur_mant_printable);
    LOG(trace) << "      " << cur_mant_printable;
    LOG(trace) << "======================================================================";

    assert(E <= 64);
    assert(M <= 64);
    assert(cur_exp >= 0);
    assert(msb(static_cast<uint128_t>(cur_exp)) <= 63);
    assert(msb(cur_mant) <= 63);

    return Flexfloat(
        E, M, B, 
        cur_sign, static_cast<uint64_t>(cur_exp), static_cast<uint64_t>(cur_mant)
    );
}

bool Flexfloat::is_valid() const
{
    if (!(M <= sizeof(m)*8))
        goto ivalid_obj;
    if (!(E <= sizeof(e)*8))
        goto ivalid_obj;
    if (!(s <= 1))
        goto ivalid_obj;
    if (!(e <= static_cast<uint64_t>((1 << E) - 1)))
        goto ivalid_obj;
    if (!(m <= static_cast<uint64_t>((1 << M) - 1)))
        goto ivalid_obj;

    LOG(trace) << "Object is valid";
    return true;

ivalid_obj:
    LOG(error) << "Object is invalid";
    return false;
}
}
