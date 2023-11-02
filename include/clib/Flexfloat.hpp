#pragma once
#include "common.hpp"

namespace clib {

/*!
 * \brief Число с плавующей запятой с настриваемой мантиссой и экспонентой.
 *
 * \details Число поддерживает денормализованные числа; нет NaN; нет ±inf
 */
class Flexfloat
{
public:
    using Stype = uint8_t;
    using Etype = uint8_t;
    using Mtype = uint8_t;
    using Btype = int;
    
    using etype = uint64_t;
    using mtype = uint64_t;
    using stype = uint8_t;

private:
    Btype B;             /// BIAS
    Etype E;             /// EXPONENT WIDTH
    Mtype M;             /// MANTISSA WIDTH
    const Stype S = 1;   /// SIGN WIDTH

    stype s; /// Sign.     Должна принадлежать [0, 1]
    etype e; /// Exponent. Должна принадлежать [0, 2^E - 1]
    mtype m; /// Mantissa. Должна принадлежать [0, 2^M - 1]

public:
    /*! @brief Создает Flexfloat
    *
    * \param[in] E_n Количество бит в экспоненте
    * \param[in] M_n Количество бит в мантиссе
    * \param[in] B_n Bias
    * \param[in] s_n Sign
    * \param[in] e_n Exponent
    * \param[in] m_n Mantissa
    */
    Flexfloat(Etype E_n, Mtype M_n, Btype B_n, stype s_n, etype e_n, mtype m_n);

    //! Generate Overflow number with s = 1; e = 2^E - 1; m = 2^M - 1
    static Flexfloat ovf(Etype E_n, Mtype M_n, Btype B_n, stype s_n);

    //! Generate Zero number with s = e = m = 0
    static Flexfloat zero(Etype E_n, Mtype M_n, Btype B_n, stype s_n);

    //! Generate minimal denormalized value = 2^(1-B-M)
    static Flexfloat min_denorm(Etype E_n, Mtype M_n, Btype B_n, stype s_n);

    //! Generate maximal denormalized value = 2^(1-B) - min_denorm
    static Flexfloat max_denorm(Etype E_n, Mtype M_n, Btype B_n, stype s_n);

    //! Generate minimal normalized value = 2^(1-B)
    static Flexfloat min_norm(Etype E_n, Mtype M_n, Btype B_n, stype s_n);

    //! Generate maximal normalized value = 2^(Emax - B) * (1 - 2^(-M))
    static Flexfloat max_norm(Etype E_n, Mtype M_n, Btype B_n, stype s_n);

    etype get_e() const noexcept { return e; }
    stype get_s() const noexcept { return s; }
    mtype get_m() const noexcept { return m; }
    Btype get_B() const noexcept { return B; }
    Mtype get_M() const noexcept { return M; }
    Etype get_E() const noexcept { return E; }

    //! \return 2^E - 1
    etype max_exp() const;
    //! \return 2^M - 1
    mtype max_mant() const;

    //! \return 2^E - 1
    static etype max_exp(Etype E);
    //! \return 2^M - 1
    static mtype max_mant(Mtype M);

    /*! @brief Ищет MSB в числе
    *
    * Пример, для val = 0b000001 вернет 0.
    * Пример, для val = 0b000010 вернет 1.
    */
    static Mtype msb(uint128_t val);

    static bool is_zero(const Flexfloat& val);


    /*! @brief Обрезает мантиссу и экспоненту числа other до
     *          размеров мантиссы и экспоненты числа this
     */
    Flexfloat& operator=(const Flexfloat& other);

    /*! @brief Умножение Flexfloat
    *
    * \param[in] left Левый операнд
    * \param[in] right Правый операнд
    * \param[in] res Результат
    */
    static void mult(const Flexfloat &left, const Flexfloat &right, Flexfloat &res);

    /// Выводит Flexfloat в информативном виде
    friend std::ostream &operator<<(std::ostream &oss, const Flexfloat &num);

    /// Выводит Flexfloat в битовом виде
    std::string bits() const;

private:

    // Обрезает мантиссу и экспоненту числа до необходимых значений
    //
    // Input: -inf <  cur_exp  < +inf
    //           0 <= cur_mant < +inf
    //
    // Desired output: Flexfloat with
    //           0 <= cur_exp  < 2^E
    //           0 <= cur_mant < 2^M
    //
    static Flexfloat normalise(uint8_t cur_sign, int128_t cur_exp, uint128_t cur_mant, Etype E, Mtype M, Btype B);

    bool is_valid() const;
};
}