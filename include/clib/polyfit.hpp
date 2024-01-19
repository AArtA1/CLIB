#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "clib/common.hpp"

namespace clib
{

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
     * \param[in] L Битовая длина входного и выходного значений
     * \param[in] L_base - Все вычисления происходит в кольце 2**L. Иначе все вычисления будут происходит
     *                     в кольце коэффициентов многочленов 2**F
     *
     * \return Результат приближения
     */
    polyfit_t calc(const std::string &fname, polyfit_t l, unsigned L, bool L_base = false)
    {
        assert(L < sizeof(polyfit_t) * 8);
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
        auto F = COEFS[fname][segment][0];

        // std::cout << "l = " << l << std::endl;
        // std::cout << "L = " << L << std::endl;
        // std::cout << "segments_bits = " << +segments_bits << std::endl;
        // std::cout << "segment = " << segment << std::endl;
        // std::cout << "F = " << F << std::endl;

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
        if (res < 0)
            res = 0;

        if (!L_base)
        {
            if (L < F)
                return static_cast<polyfit_t>(res >> (F - L));
            else
                return static_cast<polyfit_t>(res << (L - F));
        }
        return static_cast<polyfit_t>(res);
    }

  private:
    polyfit() = default;
};

} // namespace clib