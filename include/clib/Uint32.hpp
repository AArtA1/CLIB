#pragma once

#include "common.hpp"

namespace clib {

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
    explicit Uint32(uint32_t val) noexcept;

    Uint32 operator*(const Uint32 &right) const noexcept;

    Uint32 operator+(const Uint32 &right) const noexcept;

    uint32_t get_uint32_t() const noexcept;
};

}