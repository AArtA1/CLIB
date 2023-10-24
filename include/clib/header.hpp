#pragma once
#include <cstdint>
#include <iostream>
#include <cmath>
#include <bitset>

namespace clib
{
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
}
