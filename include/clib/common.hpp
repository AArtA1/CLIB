#pragma once
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

namespace clib {

//#define arg_t uint32_t

using uint128_t = __uint128_t;
using int128_t = __int128_t;

using llu_t = long long unsigned;

template <typename T>
T abs(T a) {
    if (a < 0)
        return -a;
    else
        return a;
}

template <typename T>
T delta(T a, T b) {
    if (a < b)
        return b - a;
    else
        return a - b;
}

// OUTDATED

/*
// TOOD Переписать на constexpr
template <typename T>
std::string bits(T val)
{
    if (val < 0)
        return std::to_string(static_cast<int64_t>(val));

    if (sizeof(T) > sizeof(uint64_t))
    {
        using basetype = uint64_t;
        auto val_printable  = *reinterpret_cast<basetype*>(&val);
        std::string val_bits =
std::bitset<sizeof(basetype)*8>(val_printable).to_string(); auto one_pos =
val_bits.find_first_of('1'); if (one_pos == std::string::npos) return "0";

        return val_bits.substr(one_pos) + " = " + std::to_string(val_printable);
    }
    else if (sizeof(T) > sizeof(uint32_t))
    {
        using basetype = uint32_t;
        auto val_printable  = *reinterpret_cast<basetype*>(&val);
        std::string val_bits =
std::bitset<sizeof(basetype)*8>(val_printable).to_string(); auto one_pos =
val_bits.find_first_of('1'); if (one_pos == std::string::npos) return "0";

        return val_bits.substr(one_pos) + " = " + std::to_string(val_printable);
    }
    else if (sizeof(T) > sizeof(uint16_t))
    {
        using basetype = uint16_t;
        auto val_printable  = *reinterpret_cast<basetype*>(&val);
        std::string val_bits =
std::bitset<sizeof(basetype)*8>(val_printable).to_string(); auto one_pos =
val_bits.find_first_of('1'); if (one_pos == std::string::npos) return "0";

        return val_bits.substr(one_pos) + " = " + std::to_string(val_printable);
    }

    using basetype = uint8_t;
    auto val_printable  = *reinterpret_cast<basetype*>(&val);
    std::string val_bits =
std::bitset<sizeof(basetype)*8>(val_printable).to_string(); auto one_pos =
val_bits.find_first_of('1'); if (one_pos == std::string::npos) return "0";

    return val_bits.substr(one_pos) + " = " + std::to_string(val_printable);
}
*/

/*
std::initializer_list<std::string> print(std::initializer_list<Flexfloat> list){

}
*/

/*
class bitset {
public:
    bitset(arg_t value, const size_t n) : arr_(new bool[n]), n_(n),
width_(n),to_left_side_(false){ make_bitset(value);
    }

    bitset(arg_t value,const size_t n, const size_t width, bool to_left_side) :
arr_(new bool[n]), n_(n), width_(width),to_left_side_(to_left_side){
        make_bitset(value);
    }

    ~bitset(){
        delete[] arr_;
    }

    void make_bitset(arg_t &value) const {
        for (size_t i = 0; i < n_; ++i) {
            arr_[i] = value & (static_cast<arg_t>(1) << i);
        }
    }

    arg_t convert2uint() const {
        arg_t res = 0;
        for (size_t i = 0; i < n_; ++i) {
            res = res | ((arr_[i] ? 1 : 0) << i);
        }
        return res;
    }

    friend bool operator==(const bitset &left, const bitset &right) {
        if (left.n_ != right.n_) {
            throw "Sizes of bitsets have to be the same!";
        }
        for (size_t i = 0; i < left.n_; ++i) {
            if (left.arr_[i] != right.arr_[i]) {
                return false;
            }
        }
        return true;
    }


    friend std::ostream &operator<<(std::ostream &oss, const bitset &num){
        if(!num.to_left_side_){
            for(int i = 0; i < (num.width_ - num.n_); ++i){
                oss << " ";
            }
            for (int i = num.n_ - 1; i >= 0; --i) {
                oss << num.arr_[i];
            }
        }
        else{
            for (int i = num.n_ - 1; i >= 0; --i) {
                oss << num.arr_[i];
            }
            for(int i = 0; i < (num.width_ - num.n_); ++i){
                oss << " ";
            }
        }
        return oss;
    }

private:
    bool *arr_;
    const size_t n_;
    const size_t width_;
    const bool to_left_side_;
};
*/

}  // namespace clib