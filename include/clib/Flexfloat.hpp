#pragma once
#include "common.hpp"

namespace clib
{


class Flexfixed;

/*!
 * \brief Число с плавующей запятой с настриваемой мантиссой и экспонентой.
 *
 * \details Число поддерживает денормализованные числа; нет NaN; нет ±inf
 *
 * \see gitlab.inviewlab.com/synthesizer/documents/-/blob/master/out/flexfloat.pdf
 */
class Flexfloat
{
  public:
    using Stype = uint8_t;
    using Etype = uint8_t;
    using Mtype = uint8_t;
    using Btype = int;

    using etype = uint32_t;
    using mtype = uint32_t;
    using stype = uint8_t;

    // type for storing multiplication of mantissa
    using mexttype = uint64_t;

    // type for storing sum of exp and bias
    using eexttype = int64_t;

    const static Btype B_FLOAT = 127;
    const static Etype E_FLOAT = 8;
    const static Mtype M_FLOAT = 23;

  private:
    Btype B;           /// BIAS
    Etype E;           /// EXPONENT WIDTH
    Mtype M;           /// MANTISSA WIDTH
    const Stype S = 1; /// SIGN WIDTH

    stype s; /// Sign.     Должна принадлежать [0, 1]
    etype e; /// Exponent. Должна принадлежать [0, 2^E - 1]
    mtype m; /// Mantissa. Должна принадлежать [0, 2^M - 1]

  public:
    /// @brief Создает не валидный Flexfloat
    /// synthesizer_func Flexfloat::Const
    Flexfloat();

    /*! @brief Создает Flexfloat
     *
     * \param[in] E_n Количество бит в экспоненте
     * \param[in] M_n Количество бит в мантиссе
     * \param[in] B_n Bias
     * \param[in] s_n Sign
     * \param[in] e_n Exponent
     * \param[in] m_n Mantissa
     */
    /// synthesizer_func Flexfloat::Const
    Flexfloat(Etype E_n, Mtype M_n, Btype B_n, stype s_n, etype e_n, mtype m_n);

    /*! @brief Создает Flexfloat из аналогичного битового представления
     *
     * \param[in] E_n Количество бит в экспоненте
     * \param[in] M_n Количество бит в мантиссе
     * \param[in] B_n Bias
     * \param[in] value Содержит знак, экспоненту и мантиссу в порядке s|e|m
     */
    /// synthesizer_func Flexfloat::Const
    Flexfloat(Etype E_n, Mtype M_n, Btype B_n, mtype value);

    /*! @brief Создает Flexfloat из аналогичного битового представления
     *
     * \param[in] hyperparams Число содержит E, M, B для нового Flexfloat
     * \param[in] value Содержит знак, экспоненту и мантиссу в порядке s|e|m
     */
    /// synthesizer_func Flexfloat::Const
    Flexfloat(const Flexfloat &hyperparams, mtype value);

    // /*! @brief Создает Flexfloat из double
    // *
    // * \param[in] E_n Количество бит в экспоненте
    // * \param[in] M_n Количество бит в мантиссе
    // * \param[in] B_n Bias
    // * \param[in] flt число в формате float
    // */
    // Flexfloat(Etype E_n, Mtype M_n, Btype B_n, float flt);

    Flexfloat(const Flexfloat &) = default;

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
    Flexfloat max_norm() const;

    etype get_e() const noexcept
    {
        return e;
    }
    stype get_s() const noexcept
    {
        return s;
    }
    mtype get_m() const noexcept
    {
        return m;
    }
    Btype get_B() const noexcept
    {
        return B;
    }
    Mtype get_M() const noexcept
    {
        return M;
    }
    Etype get_E() const noexcept
    {
        return E;
    }

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
    static Mtype msb(mexttype val);

    static bool is_zero(const Flexfloat &val);

    /*! @brief Обрезает мантиссу и экспоненту числа other до
     *          размеров мантиссы и экспоненты числа this
     */
    Flexfloat &operator=(const Flexfloat &other);

    /*! @brief Умножение Flexfloat
     *
     * \param[in] left Левый операнд
     * \param[in] right Правый операнд
     * \param[in] res Результат
     *
     * \see gitlab.inviewlab.com/synthesizer/documents/-/blob/master/out/flexfloat_Mult.pdf
     */
    /// synthesizer_func Flexfloat::Mult
    /// synthesizer_in left,right
    /// synthesizer_out res
    static void mult(const Flexfloat &left, const Flexfloat &right, Flexfloat &res);

    /*! @brief Сложение Flexfloat
     *
     * \param[in] left Левый операнд
     * \param[in] right Правый операнд
     * \param[in] res Результат
     *
     * \see gitlab.inviewlab.com/synthesizer/documents/-/blob/master/out/flexfloat_Add.pdf
     */
    /// synthesizer_func Flexfloat::Add
    static void sum(const Flexfloat &left, const Flexfloat &right, Flexfloat &res);

    /*! @brief Вычитание Flexfloat
     *
     * \param[in] left Левый операнд
     * \param[in] right Правый операнд
     * \param[in] res Результат
     *
     * \see gitlab.inviewlab.com/synthesizer/documents/-/blob/master/out/flexfloat_Add.pdf
     */
    /// synthesizer_func Flexfloat::Sub
    static void sub(const Flexfloat &left, const Flexfloat &right, Flexfloat &res);

    /*! @brief Получение 1/x
     *
     * \param[in] x Число для инвертирования
     * \param[in] res Результат
     *
     * \see gitlab.inviewlab.com/synthesizer/documents/-/blob/master/out/flexfloat_Inv.pdf
     */
    /// synthesizer_func Flexfloat::Inv
    static void inv(const Flexfloat &x, Flexfloat &res);

    friend bool operator>(const Flexfloat &left, const Flexfloat &right);

    friend bool operator<(const Flexfloat &left, const Flexfloat &right);

    /*! @brief Получение нормализованного числа из денормализованного
     *
     * \param[in] x Число для инвертирования
     * \param[in] res Результат
     *
     * \see gitlab.inviewlab.com/synthesizer/documents/-/blob/master/out/flexfloat.pdf
     * \see https://gitlab.inviewlab.com/synthesizer/documents/-/blob/master/src/flexfloat_nonlinear.ipynb
     */
    struct ext_ff
    {
        eexttype exp = 0;
        mexttype mant = 0;
    };
    static ext_ff get_normalized(const Flexfloat &denorm);

    /*! @brief Конвертация целого числа в FlexFloat
     *
     * \param[in] x целое число
     * \return FlexFloat
     *
     * \see https://gitlab.inviewlab.com/synthesizer/documents/-/blob/master/src/flexfloat_nonlinear.ipynb
     */
    static Flexfloat ff_from_int(Etype E, Mtype M, Btype B, int x);


    /*! @brief Округление FlexFloat до ближайшего целого числа вниз
     *
     * \return ближайшее целое число меньшее FlexFloat
     *
     * \see https://gitlab.inviewlab.com/synthesizer/documents/-/blob/master/src/flexfloat_nonlinear.ipynb
     */
    int ceil() const;

    /*! @brief Преобразует Flexfloat в float
     *
     * \return ближайшее float число
     *
     */
    /// synthesizer_func Flexfloat::ToFloat
    float to_float() const;

    /*! @brief Конвертация float числа в FlexFloat
     *
     * \param[in] flt float число
     * \return FlexFloat
     */
    /// synthesizer_func Flexfloat::FromFloat
    static Flexfloat from_float(Etype E, Mtype M, Btype B, float flt);

    /*! @brief Конвертация float числа в FlexFloat
     *
     * \param[in] flt float число
     * \return FlexFloat
     */
    /// synthesizer_func Flexfloat::FromFloat
    static Flexfloat from_float(const Flexfloat &hyperparams, float flt);

    // /*! @brief Преобразует Flexfloat в double
    // *
    // * \return ближайшее double число
    // *
    // */
    // double to_double() const;  //TODO

    /// Выводит Flexfloat в информативном виде
    friend std::ostream &operator<<(std::ostream &oss, const Flexfloat &num);

    //static void convert_fx_to_ff(const Flexfixed& value, Flexfloat& res);


    /// Выводит Flexfloat в битовом виде
    std::string bits() const;
    std::string bits(const Flexfloat &ff) const;

    inline std::string to_string_e() const
    {
        return std::bitset<sizeof(etype) * 8>(get_e()).to_string().substr(sizeof(etype) * 8 - E);
    }

    inline std::string to_string_m() const
    {
        return std::bitset<sizeof(mtype) * 8>(get_m()).to_string().substr(sizeof(mtype) * 8 - M);
    }

    inline std::pair<Etype, Mtype> get_params() const
    {
        return {E, M};
    }



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
    // See gitlab.inviewlab.com/synthesizer/documents/-/blob/master/out/flexfloat_normalize.pdf
    struct hyper_params
    {
        Etype E = 0;
        Mtype M = 0;
        Btype B = 0;
    };
    static Flexfloat normalise(stype cur_sign, eexttype cur_exp, mexttype cur_mant, Mtype curM, hyper_params res);

    // Преобразует число с расширенной мантиссой в число с обычной мантиссой
    static mtype zip(eexttype exp, mexttype ext_mant, Mtype M);
    static mtype zip(eexttype exp, mexttype ext_mant, Mtype curM, Mtype reqM);


    // Преобразует число с обычной мантиссой в число с расширенной мантиссой
    static mexttype unzip(etype exp, mtype mant, Mtype M);
    static mexttype unzip(const Flexfloat &ff);

    bool is_valid() const;
    void static check_ffs(std::initializer_list<Flexfloat> list);
};
} // namespace clib