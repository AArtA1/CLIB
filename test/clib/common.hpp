#pragma once

#include <clib/Flexfloat.hpp>
#include <clib/Uint32.hpp>
#include "clib/logs.hpp"

#include <../doctest.h>
#include <string>
#include <cstring>
#include <cassert>

#include <iostream>
#include <iomanip>   // std::setprecision, std::setw
#include <iostream>  // LOG(clib::debug), std::fixed

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions.hpp>

extern boost::log::sources::severity_logger< clib::severity_level > slg;

template <typename T>
bool check_bitwise_eq(T a, T b)
{
    return !strncmp(reinterpret_cast<char*>(&a), reinterpret_cast<char*>(&b), sizeof(T));
}