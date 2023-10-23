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
    int a = 1;
    int b = 2;

    clib::flex_int fa(a);
    clib::flex_int fb(b);

    CHECK(check_bitwise_eq(a+a*b, (fa+fa*fb).get_val()));
}
