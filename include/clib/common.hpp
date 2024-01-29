#pragma once
#include <algorithm>
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace clib
{

// #define arg_t uint32_t

using uint128_t = __uint128_t;
using int128_t = __int128_t;

using llu_t = long long unsigned;

/*! @brief Модуль
 */
template <typename T> T abs(T a)
{
    if (a < 0)
        return -a;
    else
        return a;
}

/*! @brief Модуль разницы
 */
template <typename T> T delta(T a, T b)
{
    if (a < b)
        return b - a;
    else
        return a - b;
}

/*! @brief Ищет MSB в числе
 *
 * Пример, для val = 0b000001 вернет 0.
 * Пример, для val = 0b000010 вернет 1.
 */
template <typename T> uint8_t msb(T val)
{
    if (val == 0) return 0;

    uint8_t msb = 0;
    while (val > 0)
    {
        val >>= 1;
        msb++;
    }

    return msb - 1;
}

// val < 0 -> prints integer representation of val
// val > 0 -> prints bits    representation of val
template <std::size_t N, typename... T> using static_switch = typename std::tuple_element<N, std::tuple<T...>>::type;
template <typename T> std::string bits(T val)
{
    static_assert(std::is_integral<T>::value, "Integral required.");
    if (val < 0)
        return std::to_string(static_cast<int64_t>(val));

    constexpr size_t Tsize = sizeof(T) - 1;
    using basetype =
        static_switch<Tsize, uint8_t, uint16_t, uint32_t, uint32_t, uint64_t, uint64_t, uint64_t, uint64_t>;

    auto val_printable = *reinterpret_cast<basetype *>(&val);
    std::string val_bits = std::bitset<sizeof(basetype) * 8>(val_printable).to_string();
    auto one_pos = val_bits.find_first_of('1');
    if (one_pos == std::string::npos)
        return "0";

    return val_bits.substr(one_pos) + " = " + std::to_string(val_printable);
}

template<typename T>
void debug_vecvec(std::vector<std::vector<T>> array){
    for(auto row : array){
        for(auto item : row)
            std::cout << item.to_float() << " ";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}


} // namespace clib