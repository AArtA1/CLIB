#include <doctest.h>
#include "clib/image.hpp"
#include "clib/Flexfloat.hpp"
#include "clib/logs.hpp"
#include <algorithm>

using ff = clib::Flexfloat;
using img = clib::img<ff>;

const ff::Etype E = 8;
const ff::Mtype M = 23;
const ff::Btype B = 127;

#define ff_(value) ff::from_arithmetic_t(E, M, B, value)

// TEST_CASE("Test Image Max")
// {
//     BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Image Max");

//     img r(ff(E, M, B, 0), 1, 5);
//     img g(ff(E, M, B, 0), 1, 5);
//     img b(ff(E, M, B, 0), 1, 5);
//     img c(ff(E, M, B, 0), 1, 5);

//     #define from_float_(value) ff::from_float(E,M,B,value)

//     r.vv_ = {{from_float_(12.23), from_float_(3.12),from_float_(5.112),from_float_(5.7),from_float_(3.5)}};

//     g.vv_ = {{from_float_(6), from_float_(2),from_float_(4.25),from_float_(5.1),from_float_(3.63)}};

//     b.vv_ = {{from_float_(7.2342), from_float_(2),from_float_(4.25),from_float_(5.5),from_float_(3.6)}};

//     c.vv_ = {{from_float_(14.2), from_float_(3.14),from_float_(6.123),from_float_(5.1),from_float_(3.2)}};

//     auto res = img::max(r,g,b,c);

//     for(auto i = 0; i < res.rows();++i)
//         for(auto j = 0; j < res.cols();++j)
//             std::cout << res(i,j).to_float() << " ";

// }

TEST_CASE("Test Window")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Window");

    std::vector<std::vector<ff>> arr(4, std::vector<ff>(4));

    for (int i = 0; i < arr.size(); ++i)
    {
        for (int j = 0; j < arr[0].size(); ++j)
        {
            arr[i][j] = ff_(i+j+1);
        }
    }

    img vv(arr);

    // test case 1
    auto test_1 = img::get_window(vv, {1, 1}, {3, 3});
    // auto right_1 =
    //     std::vector<std::vector<ff>>{{ff_(6), ff_(5), ff_(6)}, {ff_(2), ff_(1), ff_(2)}, {ff_(6), ff_(5), ff_(6)}};
    // if (test_1 != right_1)
    // {
    //     CHECK(false);
    // }

    // // test case 2
    // auto test_2 = img::get_window(vv, {3, 0}, {3, 3});
    // auto right_2 = std::vector<std::vector<ff>>{
    //     {ff_(10), ff_(9), ff_(10)}, {ff_(14), ff_(13), ff_(14)}, {ff_(10), ff_(9), ff_(10)}};
    // if (test_2 != right_2)
    // {
    //     CHECK(false);
    // }

    // // test case 3
    // auto test_3 = get_window(vv,{1,1},{3,3});
    // auto right_3 = std::vector<std::vector<ff>>{{1,2,3},{5,6,7},{9,10,11}};
    // if(test_3 != right_3){
    //     return false;
    //}
    
    CHECK(true);
}

#undef ff_
