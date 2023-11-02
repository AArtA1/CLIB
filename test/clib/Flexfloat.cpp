#include "common.hpp"

TEST_CASE("Test Flexfloat helpful functions")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat helpful functions");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    LOG(debug) << "max_mant(0) = " << ff::max_mant(0);
    LOG(debug) << "max_mant(1) = " << ff::max_mant(1);
    LOG(debug) << "max_mant(2) = " << ff::max_mant(2);

    LOG(debug) << "max_exp(0) = " << ff::max_exp(0);
    LOG(debug) << "max_exp(1) = " << ff::max_exp(1);
    LOG(debug) << "max_exp(2) = " << ff::max_exp(2);

    ff tmp = ff::ovf(10, 10, 10, 0);
    LOG(debug) << "ovf(10, 10, 10, 0) = " << tmp.bits();

    tmp = ff::zero(10, 10, 10, 0);
    LOG(debug) << "zero(10, 10, 10, 0) = " << tmp.bits();
    
    tmp = ff::min_denorm(10, 10, 10, 0);
    LOG(debug) << "min_denorm(10, 10, 10, 0) = " << tmp.bits();

    tmp = ff::max_denorm(10, 10, 10, 0);
    LOG(debug) << "max_denorm(10, 10, 10, 0) = " << tmp.bits();

    tmp = ff::min_norm(10, 10, 10, 0);
    LOG(debug) << "min_norm(10, 10, 10, 0) = " << tmp.bits();

    tmp = ff::max_norm(10, 10, 10, 0);
    LOG(debug) << "max_norm(10, 10, 10, 0) = " << tmp.bits();
}

TEST_CASE("Test Flexfloat mult")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat mult");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    ff a1(8, 23, 127, 0, 129, 4194304); // 6 in float
    LOG(debug) << "a in float = " << to_float(a1) << std::endl;

    ff b1(8, 23, 127, 0, 129, 6291456); // 7 in float
    LOG(debug) << "b in float = " << to_float(b1) << std::endl;

    ff c1(8, 23, 127, 1, 1, 1);

    ff::mult(a1, b1, c1);
    LOG(debug) << "result in float = " << to_float(c1) << std::endl;

    ff a2(8, 23, 127, 1, 132, 2654006); // -42.124231
    LOG(debug) << "a in float = " << to_float(a2) << std::endl;

    ff b2(8, 23, 127, 0, 142, 451618); // 34532.1324
    LOG(debug) << "b in float = " << to_float(b2) << std::endl;

    ff c2(8, 23, 127, 1, 1, 1);
    
    ff::mult(a2, b2, c2);
    LOG(debug) << "result in float = " << to_float(c2) << std::endl;
    
    CHECK(1);
}