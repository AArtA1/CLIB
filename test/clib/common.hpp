#pragma once

#include "clib/logs.hpp"

#include <cassert>
#include <cstring>
#include <string>

#include <iomanip> // std::setprecision, std::setw
#include <iostream>
#include <iostream> // LOG(clib::debug), std::fixed
#ifdef BOOST_LOGS
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#endif

#ifndef BOOST_LOGS
extern boost::log::sources::severity_logger<clib::severity_level> slg;
#endif

template <typename T> bool check_bitwise_eq(T a, T b)
{
    return !strncmp(reinterpret_cast<char *>(&a), reinterpret_cast<char *>(&b), sizeof(T));
}