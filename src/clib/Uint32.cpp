#include "clib/Uint32.hpp"
#include "clib/logs.hpp"

namespace clib {


Uint32::Uint32(uint32_t val) noexcept : val_(val)
{
}

Uint32 Uint32::operator*(const Uint32 &right) const noexcept
{
    return Uint32(val_ * right.val_);
}

Uint32 Uint32::operator+(const Uint32 &right) const noexcept
{
    return Uint32(val_ + right.val_);
}

uint32_t Uint32::get_uint32_t() const noexcept
{
    return val_;
}
}