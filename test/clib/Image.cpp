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


void print_img(img image){
    for(auto row : image.vv()){
        for(auto it : row){
            std::cout << it.to_float() << " ";
        }
        std::cout << std::endl;
    }
}


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
    std::vector<std::vector<ff>> arr(4, std::vector<ff>(4));

    int counter = 0;
    for (int i = 0; i < arr.size(); ++i)
    {
        for (int j = 0; j < arr[0].size(); ++j)
        {
            arr[i][j] = ff_(++counter);
        }
    }

    for (auto it : arr)
    {
        for (auto val : it)
        {
            std::cout << val.to_float() << " ";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;

    img vv(arr);

    // test case 1
    auto test_1 = img::get_window(vv, {1, 1}, {3, 3});
    for (auto it : test_1.vv())
    {
        for (auto val : it)
        {
            std::cout << val.to_float() << " ";
        }
        std::cout << std::endl;
    }

    auto right_1 =
        std::vector<std::vector<ff>>{{ff_(1), ff_(2), ff_(3)}, {ff_(5), ff_(6), ff_(7)}, {ff_(9), ff_(10), ff_(11)}};
    CHECK(test_1.vv() == right_1);

    std::cout << std::endl;

    // test case 2
    auto test_2 = img::get_window(vv, {0, 0}, {3, 3});

    for (auto it : test_2.vv())
    {
        for (auto val : it)
        {
            std::cout << val.to_float() << " ";
        }
        std::cout << std::endl;
    }

    auto right_2 =
        std::vector<std::vector<ff>>{{ff_(6), ff_(5), ff_(6)}, {ff_(2), ff_(1), ff_(2)}, {ff_(6), ff_(5), ff_(6)}};
    CHECK(test_2.vv() == right_2);

    // test case 3
    auto test_3 = img::get_window(vv, {3, 0}, {3, 3});

    auto right_3 = std::vector<std::vector<ff>>{
        {ff_(10), ff_(9), ff_(10)}, {ff_(14), ff_(13), ff_(14)}, {ff_(10), ff_(9), ff_(10)}};
    CHECK(test_3.vv() == right_3);
}

TEST_CASE("Test Window Overloaded"){
    std::vector<std::vector<ff>> arr(4, std::vector<ff>(4));

    int counter = 0;
    for (int i = 0; i < arr.size(); ++i)
    {
        for (int j = 0; j < arr[0].size(); ++j)
        {
            arr[i][j] = ff_(++counter);
        }
    }

    for (auto it : arr)
    {
        for (auto val : it)
        {
            std::cout << val.to_float() << " ";
        }
        std::cout << std::endl;
    }


    std::cout << std::endl;

    auto copy = arr;

    img vv(static_cast<std::vector<std::vector<ff>>&&>(arr));

    auto val = img::get_window(vv, {3, 3});

    for(auto row : val){
        for(auto ii : row){
            std::cout << std::endl;
            print_img(ii);
        }
    }

    // tested with python, works correctly
}


TEST_CASE("Test Convolution Overloaded"){
    std::vector<std::vector<ff>> arr(4, std::vector<ff>(4));

    int counter = 0;
    for (int i = 0; i < arr.size(); ++i)
    {
        for (int j = 0; j < arr[0].size(); ++j)
        {
            arr[i][j] = ff_(++counter);
        }
    }

    for (auto it : arr)
    {
        for (auto val : it)
        {
            std::cout << val.to_float() << " ";
        }
        std::cout << std::endl;
    }


    std::cout << std::endl;

    img vv(static_cast<std::vector<std::vector<ff>>&&>(arr));

    auto val = img::get_window(vv, {3, 3});
 
    img mx({
    {ff_(1.0f), ff_(0.0f), ff_(-1.0f)}, 
    {ff_(2.0f), ff_(0.0f), ff_(-2.0f)}, 
    {ff_(1.0f), ff_(0.0f), ff_(-1.0f)}});
    

    img right({
        {ff_(0.0f), ff_(-8.0f), ff_(-8.0f), ff_(0.0f)}, 
        {ff_(0.0f), ff_(-8.0f), ff_(-8.0f), ff_(0.0f)}, 
        {ff_(0.0f), ff_(-8.0f), ff_(-8.0f), ff_(0.0f)},
        {ff_(0.0f), ff_(-8.0f), ff_(-8.0f), ff_(0.0f)}
        });

    auto result_of_conv = img::convolution(mx,val);

    //std::cout << ff_(0.0f) << std::endl;

    //std::cout << ff_(0.0f).to_float();

    print_img(result_of_conv);


    //print_img(val[0][0] * mx(0,0) +  val[0][2] * mx(0,2) + mx(1,0) * val[1][0] + mx(1,2) * val[1][2] + val[2][0] * mx(2,0) + mx(2,2) * val[2][2]);    

    //CHECK(res == right);
}


TEST_CASE("Test Demosaic"){
    const img r({
        {ff_(1.0f), ff_(2.0f), ff_(3.0f), ff_(4.0f)}, 
        {ff_(5.0f), ff_(6.0f), ff_(7.0f), ff_(8.0f)}, 
        {ff_(9.0f), ff_(10.0f), ff_(11.0f), ff_(12.0f)},
        {ff_(13.0f), ff_(14.0f), ff_(15.0f), ff_(16.0f)}
        });

    const img g({
        {ff_(17.0f), ff_(18.0f), ff_(19.0f), ff_(20.0f)}, 
        {ff_(21.0f), ff_(22.0f), ff_(23.0f), ff_(24.0f)}, 
        {ff_(25.0f), ff_(26.0f), ff_(27.0f), ff_(28.0f)},
        {ff_(29.0f), ff_(30.0f), ff_(31.0f), ff_(32.0f)}
        });

    const img b({
        {ff_(33.0f), ff_(34.0f), ff_(35.0f), ff_(36.0f)}, 
        {ff_(37.0f), ff_(38.0f), ff_(39.0f), ff_(40.0f)}, 
        {ff_(41.0f), ff_(42.0f), ff_(43.0f), ff_(44.0f)},
        {ff_(45.0f), ff_(46.0f), ff_(47.0f), ff_(48.0f)}
        });

    
    auto vec_of_res = img::demosaic(r,g,b);

    print_img(vec_of_res[0]);
    print_img(vec_of_res[1]);
    print_img(vec_of_res[2]);
}

TEST_CASE("Test Convolution")
{
    std::vector<std::vector<ff>> arr(4, std::vector<ff>(4));

    int counter = 0;
    for (int i = 0; i < arr.size(); ++i)
    {
        for (int j = 0; j < arr[0].size(); ++j)
        {
            arr[i][j] = ff_(++counter);
        }
    }

    for (auto it : arr)
    {
        for (auto val : it)
        {
            std::cout << val.to_float() << " ";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;

    img vv(arr);

    std::function<ff(img)> func = [](img image) {
        auto sum = ff_(0);
        for (auto i = 0; i < image.rows(); ++i)
        {
            for (auto j = 0; j < image.cols(); ++j)
            {
                ff::sum(image(i, j), sum, sum);
            }
        }
        return sum;
    };

    auto res = img::convolution(vv, {3, 3}, func);

    for (auto i = 0; i < res.rows(); ++i)
    {
        for (auto j = 0; j < res.cols(); ++j)
        {
            std::cout << res(i, j).to_float() << " ";
        }
        std::cout << std::endl;
    }
}

TEST_CASE("Test FF"){
    auto zero = ff_(0.0f);
    std::cout << zero << " " << zero.to_float() << std::endl;

    auto zero_from_int = ff_(0);

    std::cout << zero_from_int << " " << zero_from_int.to_float(); 
}



#undef ff_
