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

TEST_CASE("Test Flexfloat sum")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat sum");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    ff a1(8, 23, 127, 0, 129, 4194304); // 6 in float
    LOG(debug) << "a in float = " << to_float(a1) << std::endl;

    ff b1(8, 23, 127, 0, 129, 6291456); // 7 in float
    LOG(debug) << "b in float = " << to_float(b1) << std::endl;

    ff c1(8, 23, 127, 1, 1, 1);

    ff::sum(a1, b1, c1);
    LOG(debug) << "result in float = " << to_float(c1) << std::endl;



    ff a2(5, 10, 15, 1028);
    //LOG(debug) << "a in float = " << to_float(a2) << std::endl;

    ff b2(5, 10, 15, 53248);
    //LOG(debug) << "b in float = " << to_float(b2) << std::endl;

    ff c2(5, 10, 15, 1);
    
    ff::sum(a2, b2, c2);
    LOG(debug) << "result\n" << c2 << std::endl;
    
    CHECK(1);
}

TEST_CASE("Test Flexfloat get_normalized")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat get_normalized");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    ff a1(4, 4, 10, 0, 0, 13);
    ff a2(4, 4, 10, 0, 0, 1);
    ff a3(4, 4, 10, 0, 0, 0);

    ff::get_normalized(a1);
    ff::get_normalized(a2);
    ff::get_normalized(a3);
   
    CHECK(1);
}


TEST_CASE("Test Flexfloat inv")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat inv");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    ff c(8, 23, 127, 1, 1, 1);

    ff a(8, 23, 127, 0, 129, 4194304); // 6 in float
    LOG(debug) << "a in float = " << to_float(a) << std::endl;
    ff::inv(a, c);
    LOG(debug) << "1 / a in float = " << to_float(c) << std::endl;

    ff b(8, 23, 127, 0, 129, 6291456); // 7 in float
    LOG(debug) << "b in float = " << to_float(b) << std::endl;
    ff::inv(b, c);
    LOG(debug) << "1 / b in float = " << to_float(c) << std::endl;

    ff d(8, 23, 127, 0, 127, 0); // 1 in float
    LOG(debug) << "d in float = " << to_float(d) << std::endl;
    ff::inv(d, c);
    LOG(debug) << "1 / d in float = " << to_float(c) << std::endl;

    ff e(8, 23, 127, 0, 0, 4194304); // small value in float
    // LOG(debug) << "e in float = " << to_float(e) << std::endl;
    ff::inv(e, c);
    LOG(debug) << "1 / e in float = " << to_float(c) << std::endl;

    CHECK(1);
}


TEST_CASE("Test Flexfloat ff_from_int")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat ff_from_int");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    auto ans = ff::ff_from_int(8, 23, 127, 1);
    if (ans.get_e() != 0) LOG(debug) << "1 in float = " << to_float(ans) << std::endl;

    ans = ff::ff_from_int(8, 23, 127, 3);
    if (ans.get_e() != 0) LOG(debug) << "3 in float = " << to_float(ans) << std::endl;

    ans = ff::ff_from_int(8, 23, 127, 10000);
    if (ans.get_e() != 0) LOG(debug) << "10000 in float = " << to_float(ans) << std::endl;

    ans = ff::ff_from_int(8, 23, 127, -7123465);
    if (ans.get_e() != 0) LOG(debug) << "-7123465 in float = " << to_float(ans) << std::endl;
    CHECK(1);
}