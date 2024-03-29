#pragma once
#include "Flexfixed.hpp"
#include "Flexfloat.hpp"
#include "common.hpp"

namespace clib
{

class Flexfloat;

/*!
 * \brief Число с фиксированной запятой.
 *
 * \details Тип данных с параметрами (I,F), целое число с прямым кодированием,
 * младшие F чисел которого считаются дробной частью. Есть поддержка overflow.
 * Нет поддержки NaN, а также +-inf .
 */
class Flexfixed
{
  public:
    using Itype = uint8_t; // size of INT_WIDTH
    using Ftype = uint8_t; // size of FRAC_WIDTH

    using stype = uint8_t;      // size of SIGN
    using ntype = uint64_t;     // size of NUMERATOR
    using nrestype = uint128_t; // size for calculating operations with ntype
    using wtype = int16_t;      // size for calculating operations with itype and ftype
  //private:
    /// INT_WIDTH
    Itype I;
    /// FRAC_WIDTH
    Ftype F;
    /// sign
    stype s;
    /// numerator
    ntype n;

  public:
    Flexfixed() = default;

    /*! @brief Создает Flexfixed
     *
     * \param[in] I_n Ширина целой части числа в битах
     * \param[in] F_n Ширина дробной части числа в битах. Количество бит
     * указывает желаемую точность после запятой.
     */
    Flexfixed(Itype I_n, Ftype F_n);

    Flexfixed(const Flexfixed &) = default;

    /// @brief Конструктор для создания Flexfixed по целочисленному значению
    /// @param I_n Ширина целочисленной части
    /// @param F_n Ширина дробной части
    /// @param val Целочисленное значение для отображения в виде Flexfixed
    Flexfixed(Itype I_n, Ftype F_n, nrestype val);

    /*! @brief Создает Flexfixed
     *
     * \param[in] I_n Ширина целочисленной части числа в битах
     * \param[in] F_n Ширина дробной части числа в битах. Количество бит
     * указывает желаемую точность после запятой. \param[in] s_n Sign - Знак
     * числа: 0 - положительное, 1 - отрицательное. \param[in] n_n Прямой код
     * представления числа, не учитывая знак числа. Последние F бит указывают на
     * дробную часть, а первые I на целую.
     */
    Flexfixed(Itype I_n, Ftype F_n, stype s_n, ntype n_n);

    /// @brief Оператор копирования Flexfixed
    /// @param other Копируемое значение
    /// @return Возвращает ссылку на новый объект
    Flexfixed &operator=(const Flexfixed &other) = default;

    /*! @brief Умножение Flexfixed
     *
     * \param[in] lhs Левый операнд
     * \param[in] rhs Правый операнд
     * \param[in] res Результат
     */
    static void mult(const Flexfixed &lhs, const Flexfixed &rhs, Flexfixed &res);

    /// @brief Взятие обратного элемента (1/x)
    /// @param val Операнд
    /// @param res Результат
    static void inv(const Flexfixed &val, Flexfixed &res);

    /// @brief Нахождение Most Significant Bit (самый крайний ненулевой бит) для
    /// n
    /// @param val Значение
    /// @return Порядковый номер бита считая с конца
    static wtype msb(const Flexfixed &val);

    // static wtype lsb(const Flexfixed& val);

    /*! @brief Сложение Flexfixed
     *
     * \param[in] lhs Левый операнд
     * \param[in] rhs Правый операнд
     * \param[in] res Результат
     */
    static void sum(const Flexfixed &lhs, const Flexfixed &rhs, Flexfixed &res);

    /*! @brief Вычитание Flexfixed
     *
     * \param[in] lhs Левый операнд
     * \param[in] rhs Правый операнд
     * \param[in] res Результат
     */
    static void sub(const Flexfixed &lhs, const Flexfixed &rhs, Flexfixed &res);

    static void log2(const Flexfixed &val, Flexfixed &res);

    static nrestype check_ovf(nrestype n, Itype I, Ftype F);

    /// @brief Метод для представления Flexfixed в виде строки
    /// (SIGN|INTEGER|FRACTIONAL)
    /// @return Результат вывода
    std::string bits() const;

    float to_float() const;

    /// @brief Метод для представления Flexfixed в виде строки
    /// (SIGN|(max_width_I
    /// - I)*' '+INTEGER|FRACTIONAL+(max_width_F - F)*' ') для более уднобного
    /// сравнения результатов выполнения операций.
    /// @param width_I Максимальная ширина целочисленной части
    /// @param width_F Максимальная ширина дробной части
    /// @return
    std::string bits(const Flexfixed &fx) const;

    /// @brief Сравнивает два Flexfixed значения
    /// @param lhs Левый операнд
    /// @param rhs Правый операнд
    /// @return Возвращает true, если левое значение больше первого, иначе
    /// false.
    friend bool operator>(const Flexfixed &lhs, const Flexfixed &rhs);

    friend bool operator>=(const Flexfixed &lhs, const Flexfixed &rhs);

    friend bool operator<(const Flexfixed &lhs, const Flexfixed &rhs);

    friend bool operator<=(const Flexfixed &lhs, const Flexfixed &rhs);

    friend bool operator==(const Flexfixed &lhs, const Flexfixed &rhs);

    friend bool operator!=(const Flexfixed &lhs, const Flexfixed &rhs);

    static void negative(const Flexfixed &val, Flexfixed &res);

    static void min(const Flexfixed &lhs, const Flexfixed &rhs, Flexfixed &res);

    static void max(const Flexfixed &lhs, const Flexfixed &rhs, Flexfixed &res);

    static void clip(const Flexfixed &a, const Flexfixed &x, const Flexfixed &b, Flexfixed &out);

    /// @brief Перегруженный оператор вывода << для представления экземпляра
    /// Flexfixed в виде строки
    ///
    /// @param oss Ссылка на выходной поток
    /// @param num Ссылка на выводимый экземпляр класса
    ///
    /// @return Возвращает ссылку на выходной поток, с записанным в нем
    /// строковым значением
    friend std::ostream &operator<<(std::ostream &oss, const Flexfixed &num);

    inline Itype get_I() const
    {
        return I;
    }

    /// @brief Getter для общего доступа к полю F - ширину дробной части
    /// @return Возвращает поле F экземпляра
    inline Ftype get_F() const
    {
        return F;
    }

    /// @brief Метод для получения строкового представления целочисленной части
    /// только нужных бит n
    /// @return I бит значения n, начиная с last - f
    inline std::string to_string_int() const
    {
        return std::bitset<sizeof(ntype) * 8>(get_integer_part()).to_string().substr(sizeof(ntype) * 8 - I, I);
    }

    /// @brief Метод для получения строкового представления дробной части только
    /// нужных бит n
    /// @return Последние F бит значения n
    inline std::string to_string_frac() const
    {
        return std::bitset<sizeof(ntype) * 8>(get_fractional_part()).to_string().substr(sizeof(ntype) * 8 - F, F);
    }

    //! \return Возвращает первые I бит от n - целые биты числа.
    inline int to_int() const
    {
        if (n >> F > std::numeric_limits<int>::max())
            throw std::runtime_error{"int overflow. Can not fit Flexfixed in int"};
        int res = static_cast<int>(n >> F);
        int sign = (s == 1) ? -1 : 0;

        return sign * res;
    }

    static void abs(const Flexfixed &val, Flexfixed &res);

    inline ntype get_integer_part() const
    {
        return ((1u << F) - 1) & n;
    }

    //! \return Возвращает последние F бит от n - дробные биты числа.
    inline ntype get_fractional_part() const
    {
        return ((1u << F) - 1) & n;
    }

    /// @brief Getter для общего доступа к знаку экземпляра класса
    /// @return Возвращает поле s экземпляра класса
    inline stype get_s() const
    {
        return s;
    }

    /// @brief Getter для общего доступа к n значению экземпляра класса
    /// @return Возвращает поле n экземпляра класса
    inline ntype get_n() const
    {
        return n;
    }

    /// @brief Getter для общего доступа к параметрам класса в виде
    /// std::pair<first,second>
    /// @return Возвращает пару I,F полей экземпляра класса
    inline std::pair<Itype, Ftype> get_params() const
    {
        return {I, F};
    }

    // todo
    friend void to_flexfixed(const Flexfloat &val, Flexfixed &res);

    static Flexfixed from_arithmetic_t(Itype I_n, Ftype F_n, float flt);
    static Flexfixed from_arithmetic_t(const Flexfixed &hyperparams, float flt);
    static void from_arithmetic_t(float flt, const Flexfixed &in, Flexfixed &out);

    static Flexfixed from_arithmetic_t(Itype I_n, Ftype F_n, long unsigned n);
    static Flexfixed from_arithmetic_t(const Flexfixed &hyperparams, long unsigned n);
    static void from_arithmetic_t(long unsigned n, const Flexfixed &in, Flexfixed &out);

    static Flexfixed from_arithmetic_t(Itype I_n, Ftype F_n, int n);
    static Flexfixed from_arithmetic_t(const Flexfixed &hyperparams, int n);
    static void from_arithmetic_t(int n, const Flexfixed &in, Flexfixed &out);

  private:
    /// @brief Принимает список значений Flexfixed, которые нужно проверить на
    /// корректность (is_valid())
    /// @param list Список Flexfixed
    static void check_fxs(std::initializer_list<Flexfixed> list);

    /// @brief Считает (2^(I+F) - 1)  максимально возможное значение числа при
    /// данной экспоненте и мантиссе.
    /// @param I_n Ширина целочисленной части
    /// @param F_n Ширина дробной части
    /// @return Возвращает максимально возможное значение n
    static ntype get_max_n(Itype I_n, Ftype F_n)
    {
        return ((1u << (I_n + F_n)) - 1);
    }

    /// @brief Проверяет корректность числа. Значение n должно быть меньше
    /// 2^(I+F), а знак s <= 1
    /// @return Возвращает флаг корректности значения
    bool is_valid() const;
};

} // namespace clib