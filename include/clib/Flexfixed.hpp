#pragma once
#include "common.hpp"

namespace clib {


/*!
 * \brief Число с фиксированной запятой.
 *  
 * \details Тип данных с параметрами (I,F), целое число с прямым кодированием, младшие F чисел которого считаются дробной частью.
 *          Есть поддержка overflow. Нет поддержки NaN, а также +-inf .
 */
class Flexfixed
{
public:
    using Itype = uint8_t; // size of INT_WIDTH 
    using Ftype = uint8_t; // size of FRAC_WIDTH

    using stype = uint8_t; // size of SIGN
    using ntype = uint64_t; // size of NUMERATOR
private:
     /// INT_WIDTH
    Itype I;
    /// FRAC_WIDTH
    Ftype F; 
    /// sign
    stype s;  
    /// numerator
    ntype n; 
public:

    /// @brief Пустой конструктор удален, так как данное значение не имеет смысла.
    Flexfixed() = delete;

    /*! @brief Создает Flexfloat
    *
    * \param[in] I_n Ширина целой части числа в битах
    * \param[in] F_n Ширина дробной части числа в битах. Количество бит указывает желаемую точность после запятой.
    */
    Flexfixed(Itype I_n,Ftype F_n);


    /*! @brief Создает Flexfixed
    *
    * \param[in] I_n Ширина целой части числа в битах
    * \param[in] F_n Ширина дробной части числа в битах. Количество бит указывает желаемую точность после запятой.
    * \param[in] s_n Sign - Знак числа: 0 - положительное, 1 - отрицательное. 
    * \param[in] n_n Прямой код представления числа, не учитывая знак числа. Последние F бит указывают на дробную часть, а первые I на целую.
    */
    Flexfixed(Itype I_n, Ftype F_n, stype s_n,ntype n_n);

    /// @brief Оператор копирования Flexfixed
    /// @param other Копируемое значение
    /// @return Возвращает ссылку на новый объект
    Flexfixed& operator=(const Flexfixed& other);

    /*! @brief Умножение Flexfixed
    *
    * \param[in] left Левый операнд
    * \param[in] right Правый операнд
    * \param[in] res Результат
    */
    static void multiplication(const Flexfixed& left, const Flexfixed & right, Flexfixed & res);


    /*! @brief Сложение Flexfixed
    *
    * \param[in] left Левый операнд
    * \param[in] right Правый операнд
    * \param[in] res Результат
    */
    static void addition(const Flexfixed& left, const Flexfixed& right, Flexfixed& res);


    /*! @brief Вычитание Flexfixed
    *
    * \param[in] left Левый операнд
    * \param[in] right Правый операнд
    * \param[in] res Результат
    */
    static void substraction(const Flexfixed& left,const Flexfixed& right, Flexfixed& res);


    /// @brief Сравнивает два Flexfixed значения 
    /// @param left Левый операнд
    /// @param right Правый операнд
    /// @return Возвращает true, если левое значение больше первого, иначе false.
    friend bool operator>(const Flexfixed& left,const Flexfixed& right);

    /// Выводит Flexfloat в информативном виде
    friend std::ostream &operator<<(std::ostream &oss, const Flexfixed &num);

    /// Проверяет корректность числа. Значение n должно быть меньше 2^(I+F)
    bool is_valid() const;

    
    inline Itype get_I() const { return I; } 

    inline Ftype get_F() const { return F; }

    //! \return Возвращает первые I бит от n - целые биты числа.
    inline ntype get_int() const { return n >> F; }

    //! \return Возвращает последние F бит от n - дробные биты числа.
    inline ntype get_frac() const{ return ((static_cast<ntype>(1) << F) - 1) & n;} 

    inline stype get_s() const{ return s; }

    //! \return Возвращает n
    inline ntype get_n() const{ return n; }

};

}