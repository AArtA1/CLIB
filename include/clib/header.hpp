#pragma once
#include <cstdint>
#include <iostream>
#include <cmath>
#include <bitset>

namespace clib
{
     /*!
        \brief
            Простейший пример собственного int.

        \details
            Реализован для тестовой сборки.
     */
    class flex_int
    {
    private:
        int val_;

    public:
        explicit flex_int(int val) noexcept :
            val_(val)
        {}

        flex_int operator* (const flex_int& right) const noexcept
        {
            return flex_int(val_ * right.val_);
        }
        
        flex_int operator+ (const flex_int& right) const noexcept
        {
            return flex_int(val_ + right.val_);
        }

        int get_val() const noexcept
        {
            return val_;
        }
    };
}
