#pragma once
#include <cstdint>
#include <iostream>
#include <cmath>
#include <bitset>
#include <string>
#include <boost/log/trivial.hpp>

namespace clib
{
    using uint128_t = __uint128_t;
    using int128_t  = __int128_t;

     /*!
        \brief
            Простейший пример собственного uint32_t.

        \details
            Реализован для тестовой сборки.
     */
    class Uint32
    {
    private:
        uint32_t val_;

    public:
        explicit Uint32(uint32_t val) noexcept :
            val_(val)
        {}

        Uint32 operator* (const Uint32& right) const noexcept
        {
            return Uint32(val_ * right.val_);
        }
        
        Uint32 operator+ (const Uint32& right) const noexcept
        {
            return Uint32(val_ + right.val_);
        }

        uint32_t get_uint32_t() const noexcept;
    };

    /*!
        \brief
            Число с плавующей запятой с настриваемой мантиссой и экспонентой.

        \details
            Число поддерживает денормализованные числа; нет NaN; нет ±inf
     */
    class Flexfloat 
    {
        int     B; // BIAS
        uint8_t E; // EXPONENT WIDTH
        uint8_t M; // MANTISSA WIDTH

        uint8_t s;  // [0, 1]
        uint64_t e; // [0, 2^E-1]
        uint64_t m; // [0, 2^M-1]

    public:
        Flexfloat(uint8_t E_n, uint8_t M_n, int B_n, uint8_t s_n, uint64_t  e_n, uint64_t m_n) :
            B(B_n),
            E(E_n),
            M(M_n),
            s(s_n),
            e(e_n),
            m(m_n)
        {
            if (!is_valid())
            {
                BOOST_LOG_TRIVIAL(error) << "Can not create object. Invalid parameters";
                throw std::string{"Can not create object. Invalid parameters"};
            }
                
            BOOST_LOG_TRIVIAL(trace) << "Object successfully created";
        }

        static bool mult(
            const Flexfloat& left, const Flexfloat& right, Flexfloat& res
        )
        {
            BOOST_LOG_TRIVIAL(trace) << "Multiplication of two numbers";

            if (!left.is_valid())
            {
                BOOST_LOG_TRIVIAL(error) << "Left operand is invalid. Can not multiplicate";
                throw std::string{"Invalid operand"};
            }
            if (!right.is_valid())
            {
                BOOST_LOG_TRIVIAL(error) << "Right operand is invalid. Can not multiplicate";
                throw std::string{"Invalid operand"};
            }
            if (!res.is_valid())
            {
                BOOST_LOG_TRIVIAL(error) << "Result operand is invalid. Can not multiplicate";
                throw std::string{"Invalid operand"};
            }

            BOOST_LOG_TRIVIAL(trace) << "Left  operand:" << std::endl << left;
            BOOST_LOG_TRIVIAL(trace) << "Right operand:" << std::endl << right;

            uint8_t nsign = left.s ^ right.s;
            
            int128_t nexp = + static_cast<int128_t>(left.e)
                            + static_cast<int128_t>(right.e)
                            + static_cast<int128_t>(res.B)
                            - static_cast<int128_t>(left.B)
                            - static_cast<int128_t>(right.B);
            
            // Largest M. All calculations will be with the largest mantissa 
            uint8_t LM = std::max({left.M, right.M, res.M});
            
            // Casting all mantissa's to LM
            uint64_t left_m  = left.m  << (LM - left.M);
            uint64_t right_m = right.m << (LM - right.M);
            uint64_t res_m   = res.m   << (LM - res.M);

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
                    nmant = 2 * left_m + (nmant >> (LM-1));
                }
            }
            else
            {
                // Same as left.e == 0 && right.e != 0
                if (right.e == 0)
                {
                    nmant = 2 * right_m + (nmant >> (LM-1));
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

            long long unsigned nexp_llu  = *reinterpret_cast<long long unsigned*>(&nexp);
            long long unsigned nmant_llu = *reinterpret_cast<long long unsigned*>(&nmant);

            BOOST_LOG_TRIVIAL(trace) << "New sign: " << nsign << std::endl;
            BOOST_LOG_TRIVIAL(trace) << "New exp:  " << std::bitset<64>(nexp_llu);
            BOOST_LOG_TRIVIAL(trace) << "          " << nexp_llu << std::endl;
            BOOST_LOG_TRIVIAL(trace) << "New mant: " << std::bitset<64>(nmant_llu);
            BOOST_LOG_TRIVIAL(trace) << "          " << nmant_llu << std::endl;

            // v0) 0 <= m < 2^LM
            // if (nmant < (1<<LM) && nexp != 0)
            // {
            //     // find MSB
            //     uint8_t msb = 0;
            //     uint128_t temp = nmant;
            //     while (temp > 0)
            //     {
            //         temp >>= 1;
            //         msb++;
            //     }
            //     if (msb != 0)
            //         msb--;

            //     uint8_t shift = 0;
            //     if (nexp < 0)
            //         shift = LM - 1 - msb;
            //     else if (nexp > 255)
            //     {
            //         BOOST_LOG_TRIVIAL(error) << "Can not create object. Invalid parameters";
            //         throw std::string{"Can not create object. Invalid parameters"};
            //     }
            //         shift = LM - 1 - std::max(msb, nexp);
            // }

            return 0;
        }
    
        friend std::ostream& operator<< (std::ostream& oss, const Flexfloat & num)
        {
            oss << "M: " << static_cast<int>(num.M) << std::endl;  
            oss << "E: " << static_cast<int>(num.E) << std::endl;  
            oss << "B: " << num.B << std::endl;   

            std::string sign_s = std::bitset<8>  (num.s).to_string();
            std::string exp_s  = std::bitset<128>(num.e).to_string();
            std::string mant_s = std::bitset<128>(num.m).to_string();

            sign_s = sign_s.substr(8-1,      std::string::npos);
            exp_s  = exp_s.substr (64-num.E, std::string::npos);
            mant_s = mant_s.substr(64-num.M, std::string::npos);

            oss << "Sign: " << sign_s << std::endl;  
            oss << "Exp:  " << exp_s  << std::endl;  
            oss << "Mant: " << mant_s << std::endl;

            return oss;
        }

    private:
        bool is_valid() const
        {
            if ( !(s <= 1) )
                goto ivalid_obj;
            if ( !(e <= static_cast<uint64_t>((1 << E) - 1) ) )
                goto ivalid_obj;
            if ( !(m <= static_cast<uint64_t>((1 << M) - 1) ) )
                goto ivalid_obj;
            
            BOOST_LOG_TRIVIAL(trace) << "Object is valid";
            return true;
            
        ivalid_obj:
            BOOST_LOG_TRIVIAL(error) << "Object is invalid";
            return false;
        }
    };


}
