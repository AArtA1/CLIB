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

float to_float(const clib::Flexfloat& flex)
{
    assert(flex.get_M() == 23);
    assert(flex.get_B() == 127);
    assert(flex.get_E() == 8);
    
    // working only with normalised numbers
    assert(flex.get_e() != 0);

    std::bitset<1>  s_bits(flex.get_s());
    std::bitset<8>  e_bits(flex.get_e());
    std::bitset<23> m_bits(flex.get_m());

    std::string ans_str = s_bits.to_string() + e_bits.to_string() + m_bits.to_string();
    std::bitset<32> ans_bits(ans_str);
    unsigned long ans_ul = ans_bits.to_ulong();

    return *reinterpret_cast<float*>(&ans_ul);
}