#include <clib/image.hpp>
#include <clib/Flexfloat.hpp>
#include "clib/logs.hpp"

using ff = clib::Flexfloat;
using img = clib::img<ff>;

const ff::Etype E = 8;
const ff::Mtype M = 23;
const ff::Btype B = 127;

#define from_float_(value) ff::from_float(E,M,B,value)


TEST_CASE("Test Image Max")
{
    BOOST_LOG_SCOPED_THREAD_TAG("Tag", "Image Max");
    
    img r(ff(E, M, B, 0), 1, 5);
    img g(ff(E, M, B, 0), 1, 5);
    img b(ff(E, M, B, 0), 1, 5);
    img c(ff(E, M, B, 0), 1, 5);

    #define from_float_(value) ff::from_float(E,M,B,value)

    r.vv_ = {{from_float_(12.23), from_float_(3.12),from_float_(5.112),from_float_(5.7),from_float_(3.5)}};

    g.vv_ = {{from_float_(6), from_float_(2),from_float_(4.25),from_float_(5.1),from_float_(3.63)}};

    b.vv_ = {{from_float_(7.2342), from_float_(2),from_float_(4.25),from_float_(5.5),from_float_(3.6)}};
    
    c.vv_ = {{from_float_(14.2), from_float_(3.14),from_float_(6.123),from_float_(5.1),from_float_(3.2)}};

    auto res = img::maxxxx(r,g,b,c);

    for(auto i = 0; i < res.rows();++i)
        for(auto j = 0; j < res.cols();++j)
            std::cout << res(i,j).to_float() << " ";

}


#undef from_float_(value)
