#include <clib/Flexfloat.hpp>
#include <clib/Uint32.hpp>
#include "clib/logs.hpp"

#include <doctest/doctest.h>
#include <string>
#include <cstring>
#include <cassert>

#include <iostream>
#include <iomanip>   // std::setprecision, std::setw
#include <iostream>  // LOG(clib::debug), std::fixed

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/expressions.hpp>

boost::log::sources::severity_logger< clib::severity_level > slg;

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

TEST_CASE("Test Flexfloat")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat");

    LOG(clib::debug) << std::fixed << std::setprecision(5);

    clib::Flexfloat a1(8, 23, 127, 0, 129, 4194304); // 6 in float
    LOG(clib::debug) << "a in float = " << to_float(a1) << std::endl;

    clib::Flexfloat b1(8, 23, 127, 0, 129, 6291456); // 7 in float
    LOG(clib::debug) << "b in float = " << to_float(b1) << std::endl;

    clib::Flexfloat c1(8, 23, 127, 1, 1, 1);

    clib::Flexfloat::mult(a1, b1, c1);
    LOG(clib::debug) << "result in float = " << to_float(c1) << std::endl;

    clib::Flexfloat a2(8, 23, 127, 1, 132, 2654006); // -42.124231
    LOG(clib::debug) << "a in float = " << to_float(a2) << std::endl;

    clib::Flexfloat b2(8, 23, 127, 0, 142, 451618); // 34532.1324
    LOG(clib::debug) << "b in float = " << to_float(b2) << std::endl;

    clib::Flexfloat c2(8, 23, 127, 1, 1, 1);
    
    clib::Flexfloat::mult(a2, b2, c2);
    LOG(clib::debug) << "result in float = " << to_float(c2) << std::endl;
    
    CHECK(1);
}