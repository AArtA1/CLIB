#pragma once
#include <cstdint>
#include <iostream>
#include <cmath>
#include <bitset>
#include <string>
#include <cassert>
#include <ostream>
#include <fstream>

namespace clib {

using uint128_t = __uint128_t;
using int128_t  = __int128_t;

using llu_t = long long unsigned;

template <typename T>
T abs(T a)
{
    if (a < 0)
        return -a;
    else
        return a;
}

template <typename T>
T delta(T a, T b)
{
    if (a < b)
        return b-a;
    else
        return a-b;
}

// TOOD Переписать на constexpr
template <typename T>
std::string bits(T val)
{
    if (sizeof(T) > sizeof(uint64_t))
    {
        using basetype = uint64_t;
        auto val_printable  = *reinterpret_cast<basetype*>(&val);
        std::string val_bits = std::bitset<sizeof(basetype)*8>(val_printable).to_string();
        auto one_pos = val_bits.find_first_of('1');
        if (one_pos == std::string::npos)
            return "0";
            
        return val_bits.substr(one_pos) + " = " + std::to_string(val_printable);
    }
    else if (sizeof(T) > sizeof(uint32_t))
    {
        using basetype = uint32_t;
        auto val_printable  = *reinterpret_cast<basetype*>(&val);
        std::string val_bits = std::bitset<sizeof(basetype)*8>(val_printable).to_string();
        auto one_pos = val_bits.find_first_of('1');
        if (one_pos == std::string::npos)
            return "0";
            
        return val_bits.substr(one_pos) + " = " + std::to_string(val_printable);
    }
    else if (sizeof(T) > sizeof(uint16_t))
    {
        using basetype = uint16_t;
        auto val_printable  = *reinterpret_cast<basetype*>(&val);
        std::string val_bits = std::bitset<sizeof(basetype)*8>(val_printable).to_string();
        auto one_pos = val_bits.find_first_of('1');
        if (one_pos == std::string::npos)
            return "0";
            
        return val_bits.substr(one_pos) + " = " + std::to_string(val_printable);
    }

    using basetype = uint8_t;
    auto val_printable  = *reinterpret_cast<basetype*>(&val);
    std::string val_bits = std::bitset<sizeof(basetype)*8>(val_printable).to_string();
    auto one_pos = val_bits.find_first_of('1');
    if (one_pos == std::string::npos)
        return "0";
        
    return val_bits.substr(one_pos) + " = " + std::to_string(val_printable);
}

}