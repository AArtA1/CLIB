#include <clib/header.hpp>
#include <doctest/doctest.h>
#include <iostream>
#include <cstring>

template <typename T>
bool check_bitwise_eq(T a, T b)
{
    return !strncmp(reinterpret_cast<char*>(&a), reinterpret_cast<char*>(&b), sizeof(T));
}

TEST_CASE("Simple tests for clib")
{
    uint32_t a = 1;
    uint32_t b = 2;

    clib::Uint32 fa(a);
    clib::Uint32 fb(b);

    CHECK(check_bitwise_eq(a+a*b, (fa+fa*fb).get_uint32_t()));
}
