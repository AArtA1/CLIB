#include "common.hpp"

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