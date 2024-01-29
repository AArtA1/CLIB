#include "clib/logs.hpp"
#include <clib/Flexfloat.hpp>
#include <doctest.h>


#ifdef DEPRECATED

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
    LOG(debug) << "a in float = " << a1.to_float() << std::endl;

    ff b1(8, 23, 127, 0, 129, 6291456); // 7 in float
    LOG(debug) << "b in float = " << b1.to_float() << std::endl;

    ff c1(8, 23, 127, 1, 1, 1);

    ff::mult(a1, b1, c1);
    LOG(debug) << "result in float = " << c1.to_float() << std::endl;

    ff a2(8, 23, 127, 1, 132, 2654006); // -42.124231
    LOG(debug) << "a in float = " << a2.to_float() << std::endl;

    ff b2(8, 23, 127, 0, 142, 451618); // 34532.1324
    LOG(debug) << "b in float = " << b2.to_float() << std::endl;

    ff c2(8, 23, 127, 1, 1, 1);

    ff::mult(a2, b2, c2);
    LOG(debug) << "result in float = " << c2.to_float() << std::endl;

    CHECK(1);
}

TEST_CASE("Test Flexfloat sum")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat sum");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    ff a1(8, 23, 127, 0, 129, 4194304); // 6 in float
    LOG(debug) << "a in float = " << a1.to_float() << std::endl;

    ff b1(8, 23, 127, 0, 129, 6291456); // 7 in float
    LOG(debug) << "b in float = " << b1.to_float() << std::endl;

    ff c1(8, 23, 127, 1, 1, 1);

    ff::sum(a1, b1, c1);
    LOG(debug) << "result in float = " << c1.to_float() << std::endl;

    ff a2(5, 10, 15, 1028);
    // LOG(debug) << "a in float = " << a2.to_float() << std::endl;

    ff b2(5, 10, 15, 53248);
    // LOG(debug) << "b in float = " << b2.to_float() << std::endl;

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
    LOG(debug) << "a in float = " << a.to_float() << std::endl;
    ff::inv(a, c);
    LOG(debug) << "1 / a in float = " << c.to_float() << std::endl;

    ff b(8, 23, 127, 0, 129, 6291456); // 7 in float
    LOG(debug) << "b in float = " << b.to_float() << std::endl;
    ff::inv(b, c);
    LOG(debug) << "1 / b in float = " << c.to_float() << std::endl;

    ff d(8, 23, 127, 0, 127, 0); // 1 in float
    LOG(debug) << "d in float = " << d.to_float() << std::endl;
    ff::inv(d, c);
    LOG(debug) << "1 / d in float = " << c.to_float() << std::endl;

    ff e(8, 23, 127, 0, 0, 4194304); // small value in float
    LOG(debug) << "e in float = " << e.to_float() << std::endl;
    ff::inv(e, c);
    LOG(debug) << "1 / e in float = " << c.to_float() << std::endl;

    ff f(5, 10, 15, 1, 0, 1023); // small value in float
    ff::inv(f, c);
    LOG(debug) << "1 / f in float = " << c.to_float() << std::endl;

    CHECK(1);
}


TEST_CASE("Test Flexfloat ceil")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat ceil");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    ff a(8, 23, 127, 1, 129, 400000);
    int ceiled = a.ceil();
    LOG(debug) << "a in float = " << a.to_float();
    LOG(debug) << "a ceiled = " << ceiled << std::endl;

    CHECK(1);
}

TEST_CASE("Test Flexfloat float")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat float");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    float fa = 3.5;
    auto a = ff::from_float(8, 23, 127, fa);
    if (a.get_e() != 0)
        LOG(debug) << "3.5 in float = " << a.to_float() << std::endl;

    fa = 3.5;
    auto b = ff::from_float(5, 10, 15, fa);
    if (b.get_e() != 0)
        LOG(debug) << "3.5 in float = " << b.to_float() << std::endl;

    fa = 3.5;
    auto c = ff::from_float(4, 3, 7, fa);
    if (c.get_e() != 0)
        LOG(debug) << "3.5 in float = " << c.to_float() << std::endl;

    CHECK(1);
}

TEST_CASE("Test Flexfloat cmp")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat cmp");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    const ff::Mtype E = 8;
    const ff::Etype M = 23;
    const ff::Btype B = 127;

    auto fm2 = ff::from_float(E, M, B, -2);
    auto fm1 = ff::from_float(E, M, B, -1);
    auto f0 = ff::from_float(E, M, B, 0);
    auto f1 = ff::from_float(E, M, B, 1);
    auto f2 = ff::from_float(E, M, B, 2);
    LOG(debug) << "-2  < -1: " << (fm2 < fm1);
    LOG(debug) << "-1  < -2: " << (fm1 < fm2);
    LOG(debug) << " 2  <  1: " << (f2 < f1);
    LOG(debug) << " 1  <  2: " << (f1 < f2);
    LOG(debug) << "-2 == -1: " << (fm2 == fm1);
    LOG(debug) << "-2  >  1: " << (fm2 > f1);
    LOG(debug) << " 0  >  1: " << (f0 > f1);
    LOG(debug) << " 0  > -1: " << (f0 > fm1);

    CHECK(1);
}

TEST_CASE("Test Flexfloat clip")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat clip");
    LOG(debug) << std::fixed << std::setprecision(5);
    using ff = clib::Flexfloat;

    const ff::Mtype E = 8;
    const ff::Etype M = 23;
    const ff::Btype B = 127;

    auto f0 = ff::from_float(E, M, B, 0);
    auto fm5 = ff::from_float(E, M, B, -5);
    auto f5 = ff::from_float(E, M, B, 5);
    auto fm2 = ff::from_float(E, M, B, -2);
    auto f2 = ff::from_float(E, M, B, 2);
    auto res = ff::from_float(E, M, B, 0);
    ff::clip(f0, f5, f2, res);
    LOG(debug) << "clip(0, 5, 2) = " << res.to_float();
    ff::clip(f0, f5, fm2, res);
    LOG(debug) << "clip(0, 5, -2) = " << res.to_float();
    ff::clip(fm5, f0, f2, res);
    LOG(debug) << "clip(-5, 0, 2) = " << res.to_float();
    ff::clip(fm5, f0, fm2, res);
    LOG(debug) << "clip(-5, 0, -2) = " << res.to_float();

    CHECK(1);
}

TEST_CASE("Test Flexfloat mean")
{
    // BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Flexfloat mean");
    // LOG(debug) << std::fixed << std::setprecision(5);
    // using ff = clib::Flexfloat;

    // using std::vector;
    // const ff::Etype E = 5;
    // const ff::Mtype M = 10;
    // const ff::Btype B = 15;
    // const size_t SIZE = 1000;

    // vector<vector<ff>> img(SIZE, vector<ff>(SIZE, ff(E, M, B, 0)));
    // for (size_t i = 0; i < SIZE; ++i)
    //     for (size_t j = 0; j < SIZE; ++j)
    //         img[i][j] = ff::from_arithmetic_t(E, M, B, i * j) ;

    // LOG(debug) << "mean = " << clib::mean<ff>(img).to_float();

    CHECK(1);
}
#endif

TEST_CASE("Test Flexfloat inv")
{
    using ff = clib::Flexfloat;

    auto nums = {
        ff(8, 23, 127, 0, 129, 4194304), // 6 in float
        ff(8, 23, 127, 0, 129, 6291456), // 7 in float
        ff(8, 23, 127, 0, 127, 0),       // 1 in float
        ff(8, 23, 127, 0, 0, 4194304),   // small value in float
        ff(5, 10, 15, 1, 0, 1023),       // small value in float
        ff(8, 23, 127, 1, 120, ff::max_mant(23)),
    };

    ff c(8, 23, 127, 1, 1, 1);
    for (auto &num : nums)
    {
        std::cout << "x in float = " << num.to_float() << std::endl;
        ff::inv(num, c, 1);
        std::cout << "1/x in float = " << c.to_float() << std::endl << std::endl;
    }

    CHECK(1);
}

TEST_CASE("Test Flexfloat integer_part")
{
    using ff = clib::Flexfloat;

    auto nums = {
        ff(8, 23, 127, 0, 129, 4194304), // 6 in float
        ff(8, 23, 127, 0, 129, 6291456), // 7 in float
        ff(8, 23, 127, 0, 127, 0),       // 1 in float
        ff(8, 23, 127, 0, 140, 4994304), // big value 1
        ff(8, 23, 127, 1, 140, 6004304), // big value 2
        ff(8, 23, 127, 0, 0, 4194304),   // small value in float
        ff(5, 10, 15, 1, 0, 1023),       // small value in float
        ff(8, 23, 127, 1, 120, ff::max_mant(23)),
    };

    ff c(8, 23, 127, 1, 1, 1);
    for (auto &num : nums)
    {
        std::cout << "x in float = " << num.to_float() << std::endl;
        std::cout << "integer part = " << num.integer_part() << std::endl << std::endl;
    }

    CHECK(1);
}

TEST_CASE("Test Flexfloat fractional_part")
{
    using ff = clib::Flexfloat;

    auto nums = {
        ff(8, 23, 127, 0, 129, 4194304), // 6 in float
        ff(8, 23, 127, 0, 129, 6291456), // 7 in float
        ff(8, 23, 127, 0, 127, 0),       // 1 in float
        ff(8, 23, 127, 0, 140, 4994304), // big value 1
        ff(8, 23, 127, 1, 140, 6004304), // big value 2
        ff(8, 23, 127, 0, 0, 4194304),   // small value in float
        ff(5, 10, 15, 1, 0, 1023),       // small value in float
        ff(8, 23, 127, 1, 120, ff::max_mant(23)),
    };

    ff c(8, 23, 127, 1, 1, 1);
    for (auto &num : nums)
    {
        std::cout << "x in float = " << num.to_float() << std::endl;
        std::cout << "fractional_part = " << num.fractional_part(20) * 1.0 / (1 << 20) << std::endl << std::endl;
    }

    CHECK(1);
}

TEST_CASE("Test Flexfloat exp2")
{
    using ff = clib::Flexfloat;

    auto nums = {
        ff(8, 23, 127, 0, 0, 0),         // 0 in float
        ff(8, 23, 127, 0, 129, 4194304), // 6 in float
        ff(8, 23, 127, 0, 129, 6291456), // 7 in float
        ff(8, 23, 127, 0, 127, 0),       // 1 in float
        ff(8, 23, 127, 0, 140, 4994304), // big value 1
        ff(8, 23, 127, 1, 140, 6004304), // big value 2
        ff(8, 23, 127, 0, 0, 4194304),   // small value in float
        ff(5, 10, 15, 1, 0, 1023),       // small value in float
        ff(8, 23, 127, 1, 120, ff::max_mant(23)),
    };

    ff c(8, 23, 127, 1, 1, 1);
    for (auto &num : nums)
    {
        std::cout << "x in float = " << num.to_float() << std::endl;
        ff::exp2(num, c);
        std::cout << "e**x in float = " << c.to_float() << std::endl << std::endl;
    }

    CHECK(1);
}

TEST_CASE("Test Flexfloat log2")
{
    using ff = clib::Flexfloat;

    auto nums = {
        ff(8, 23, 127, 0, 0, 0),         // 0 in float
        ff(8, 23, 127, 0, 129, 4194304), // 6 in float
        ff(8, 23, 127, 0, 129, 6291456), // 7 in float
        ff(8, 23, 127, 0, 127, 0),       // 1 in float
        ff(8, 23, 127, 0, 140, 4994304), // big value 1
        ff(8, 23, 127, 0, 140, 6004304), // big value 2
        ff(8, 23, 127, 0, 0, 4194304),   // small value in float
        ff(5, 10, 15, 0, 0, 1023),       // small value in float
        ff(8, 23, 127, 0, 120, ff::max_mant(23)),
    };

    ff c(8, 23, 127, 1, 1, 1);
    for (auto &num : nums)
    {
        std::cout << "x in float = " << num.to_float() << std::endl;
        ff::log2(num, c);
        std::cout << "log2(x) in float = " << c.to_float() << std::endl << std::endl;
    }

    CHECK(1);
}

TEST_CASE("Test Flexfloat sqrt")
{
    using ff = clib::Flexfloat;

    auto nums = {
        ff(8, 23, 127, 0, 0, 0),         // 0 in float
        ff(8, 23, 127, 0, 129, 4194304), // 6 in float
        ff(8, 23, 127, 0, 129, 6291456), // 7 in float
        ff(8, 23, 127, 0, 127, 0),       // 1 in float
        ff(8, 23, 127, 0, 140, 4994304), // big value 1
        ff(8, 23, 127, 0, 140, 6004304), // big value 2
        ff(8, 23, 127, 0, 150, 6004304), // big value 3
        ff(8, 23, 127, 0, 0, 4194304),   // small value in float
        ff(5, 10, 15, 0, 0, 1023),       // small value in float
        ff(8, 23, 127, 0, 120, ff::max_mant(23)),
    };

    ff c(8, 23, 127, 1, 1, 1);
    for (auto &num : nums)
    {
        std::cout << "x in float = " << num.to_float() << std::endl;
        ff::sqrt(num, c);
        std::cout << "sqrt(x) in float = " << c.to_float() << std::endl << std::endl;
    }

    CHECK(1);
}

TEST_CASE("Test Flexfloat cos")
{
    using ff = clib::Flexfloat;

    auto nums = {
        ff(8, 23, 127, 0, 0, 0),         // 0 in float
        ff(8, 23, 127, 0, 129, 4194304), // 6 in float
        ff(8, 23, 127, 0, 129, 6291456), // 7 in float
        ff(8, 23, 127, 0, 130, 0),       // 8 in float
        ff(8, 23, 127, 0, 130, 1048576), // 9 in float
        ff(8, 23, 127, 0, 130, 2097152), // 10 in float
        ff(8, 23, 127, 0, 130, 3145728), // 11 in float
        ff(8, 23, 127, 0, 127, 0),       // 1 in float
        ff(8, 23, 127, 0, 140, 4994304), // big value 1
        ff(8, 23, 127, 1, 140, 6004304), // big value 2
        ff(8, 23, 127, 0, 0, 4194304),   // small value in float
        ff(5, 10, 15,  0, 0, 1023),      // small value in float
        ff(8, 23, 127, 0, 120, ff::max_mant(23)),
    };

    ff c(8, 23, 127, 1, 1, 1);
    for (auto &num : nums)
    {
        std::cout << "x in float = " << num.to_float() << std::endl;
        ff::cos(num, c);
        std::cout << "cos(x) in float = " << c.to_float() << std::endl << std::endl;
    }

    CHECK(1);
}

TEST_CASE("Test Flexfloat sin")
{
    using ff = clib::Flexfloat;

    auto nums = {
        ff(8, 23, 127, 0, 0, 0),         // 0 in float
        ff(8, 23, 127, 0, 129, 4194304), // 6 in float
        ff(8, 23, 127, 0, 129, 6291456), // 7 in float
        ff(8, 23, 127, 0, 130, 0),       // 8 in float
        ff(8, 23, 127, 0, 130, 1048576), // 9 in float
        ff(8, 23, 127, 0, 130, 2097152), // 10 in float
        ff(8, 23, 127, 0, 130, 3145728), // 11 in float
        ff(8, 23, 127, 0, 127, 0),       // 1 in float
        ff(8, 23, 127, 0, 140, 4994304), // big value 1
        ff(8, 23, 127, 1, 140, 6004304), // big value 2
        ff(8, 23, 127, 0, 0, 4194304),   // small value in float
        ff(5, 10, 15,  0, 0, 1023),      // small value in float
        ff(8, 23, 127, 0, 120, ff::max_mant(23)),
    };

    ff c(8, 23, 127, 1, 1, 1);
    for (auto &num : nums)
    {
        std::cout << "x in float = " << num.to_float() << std::endl;
        ff::sin(num, c);
        std::cout << "sin(x) in float = " << c.to_float() << std::endl << std::endl;
    }

    CHECK(1);
}

TEST_CASE("Test Flexfloat ctan")
{
    using ff = clib::Flexfloat;

    auto nums = {
        ff(8, 23, 127, 0, 0, 0),         // 0 in float
        ff(8, 23, 127, 0, 129, 4194304), // 6 in float
        ff(8, 23, 127, 0, 129, 6291456), // 7 in float
        ff(8, 23, 127, 0, 130, 0),       // 8 in float
        ff(8, 23, 127, 0, 130, 1048576), // 9 in float
        ff(8, 23, 127, 0, 130, 2097152), // 10 in float
        ff(8, 23, 127, 0, 130, 3145728), // 11 in float
        ff(8, 23, 127, 1, 129, 4194304), // -6 in float
        ff(8, 23, 127, 1, 129, 6291456), // -7 in float
        ff(8, 23, 127, 1, 130, 0),       // -8 in float
        ff(8, 23, 127, 1, 130, 1048576), // -9 in float
        ff(8, 23, 127, 1, 130, 2097152), // -10 in float
        ff(8, 23, 127, 1, 130, 3145728), // -11 in float
        ff(8, 23, 127, 0, 127, 0),       // 1 in float
        ff(8, 23, 127, 0, 140, 4994304), // big value 1
        ff(8, 23, 127, 1, 140, 6004304), // big value 2
        ff(8, 23, 127, 0, 0, 4194304),   // small value in float
        ff(5, 10, 15,  0, 0, 1023),      // small value in float
        ff(8, 23, 127, 0, 120, ff::max_mant(23)),
    };

    ff c(8, 23, 127, 1, 1, 1);
    for (auto &num : nums)
    {
        std::cout << "x in float = " << num.to_float() << std::endl;
        ff::ctan(num, c);
        std::cout << "ctan(x) in float = " << c.to_float() << std::endl;
        std::cout << "real ctan(x)     = " << 1 / std::tan(num.to_float()) << std::endl << std::endl;
    }

    CHECK(1);
}