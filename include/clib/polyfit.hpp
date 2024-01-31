#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "clib/common.hpp"

namespace clib
{

#ifdef BOOST_LOGS
#define $(...) __VA_ARGS__
#else
#define $(...) ;
#endif


// Generated file with polynomials coefficients with structure:
// {
//      name: {
//              {segment_1_bitlen, segment_1_coef_n, segment_1_coef_n-1, ...},
//              {segment_2_bitlen, segment_2_coef_n, segment_1_coef_n-1, ...}, ...},
//      ...
// }
// name - interpolated function name
// segment_i_bitlen - bit length of segment number i coefficients
// segment_i_coef_j - coefficient number j in segment number i
using std::map;
using std::string;
using std::vector;
using coef_t = int64_t;
using polyfit_t = uint64_t;
#include "../configs/polyfit_coeffs.hpp"

/*!
 * \brief Класс для подсчёта приближенных значений фунцкий через полиномиальную сплайн-интерполяцию
 */
struct polyfit final
{
    /// @brief Удаляем копирующий конструктор для экземпляра класса, так как он является синглтоном
    polyfit(polyfit const &) = delete;

    /// @brief Удаляем копирующий оператор для экземпляра класса, так как он является синглтоном
    polyfit &operator=(polyfit const &) = delete;

    ~polyfit() = default;

    /// @brief Статический метод для вызова единственного экземпляра класса
    /// @return Указатель на экземпляр класса
    static polyfit *get()
    {
        static polyfit instance{};
        return &instance;
    }

    /*! @brief Подсчёт приближённого значения функции
     *
     * \param[in] fname Название функции
     * \param[in] l Входное значение
     * \param[in] L Битовая длина входного значения
     * \param[in] K Битовая длина выходного значения
     * \param[in] L_base - Все вычисления происходит в кольце 2**L. Иначе все вычисления будут происходит
     *                     в кольце коэффициентов многочленов 2**F
     *
     * \return Результат приближения
     */
    polyfit_t calc(const std::string &fname, polyfit_t l, unsigned L, unsigned K, bool L_base = false)
    {
#ifdef BOOST_LOGS
        CLOG(trace) << std::endl;
        CLOG(trace) << "====== polyfit ======";
#endif
        assert(L < sizeof(polyfit_t) * 8);
        assert(K < sizeof(polyfit_t) * 8);
        assert(l < std::numeric_limits<coef_t>::max());

        coef_t lc = static_cast<coef_t>(l);

        // clip
        if (l >= (1u << L))
            l = (1u << L) - 1u;

        if (COEFS.find(fname) == COEFS.end())
            throw std::runtime_error{"Unknown function name: " + fname};

        auto segments = COEFS[fname].size();
        auto segments_bits = msb(segments);

        // The number of segments must be a power of 2
        assert(segments - (1 << segments_bits) == 0);

        auto segment = l >> (L - segments_bits);

        // Bit length of coefficients
        assert(COEFS[fname][segment][0] > 0);
        auto F = static_cast<unsigned>(COEFS[fname][segment][0]);

#ifdef BOOST_LOGS
        CLOG(trace) << "l = " << l;
        CLOG(trace) << "L, K, F = " << L << " " << K << " " << F;
        CLOG(trace) << "segment = " << segment;
        CLOG(trace) << "=====================";
#endif
        coef_t res = COEFS[fname][segment][1];
        if (L_base)
        {
            if (L < F)
                res >>= F - L;
            else
                res <<= L - F;
        }

        // Gorner scheme
        for (size_t i = 2; i < COEFS[fname][segment].size(); ++i)
        {
            if (L_base)
            {
                res = (res * lc) >> L;
                if (L < F)
                    res += COEFS[fname][segment][i] >> (F - L);
                else
                    res += COEFS[fname][segment][i] << (L - F);
            }
            else
            {
                res = (res * lc) >> L;
                res += COEFS[fname][segment][i];
            }
        }

        $(CLOG(trace) << "res = " << res);

        if (res < 0)
            res = 0;

        if (L_base)
        {
            if (K > L)
                return static_cast<polyfit_t>(res << (K - L));
            else
                return static_cast<polyfit_t>(res >> (L - K));
        }
        else
        {
            if (K > F)
                return static_cast<polyfit_t>(res << (K - F));
            else
                return static_cast<polyfit_t>(res >> (F - K));
        }
    }

  private:
    polyfit() = default;
};

} // namespace clib