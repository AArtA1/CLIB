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

}