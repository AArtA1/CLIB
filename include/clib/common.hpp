#pragma once
#include "logs.hpp"
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
#include <algorithm>
#include <vector>

namespace clib
{

// #define arg_t uint32_t

using uint128_t = __uint128_t;
using int128_t = __int128_t;

using llu_t = long long unsigned;

template <typename T> T abs(T a)
{
    if (a < 0)
        return -a;
    else
        return a;
}

template <typename T> T delta(T a, T b)
{
    if (a < b)
        return b - a;
    else
        return a - b;
}

template <std::size_t N, typename... T> using static_switch = typename std::tuple_element<N, std::tuple<T...>>::type;

template <typename T> std::string bits(T val)
{
    if (val < 0)
        return std::to_string(static_cast<int64_t>(val));

    constexpr size_t Tsize = sizeof(T) - 1;
    using basetype =
        static_switch<Tsize, uint8_t, uint16_t, uint32_t, uint32_t, uint64_t, uint64_t, uint64_t, uint64_t>;

    auto val_printable = *reinterpret_cast<basetype *>(&val);
    std::string val_bits = std::bitset<sizeof(basetype) * 8>(val_printable).to_string();
    auto one_pos = val_bits.find_first_of('1');
    if (one_pos == std::string::npos)
        return "0";

    return val_bits.substr(one_pos) + " = " + std::to_string(val_printable);
}

static size_t determine_threads(size_t req_threads)
{
    size_t hard_conc = static_cast<size_t>(std::thread::hardware_concurrency());
    return std::min(hard_conc != 0 ? hard_conc : 2, req_threads);
}

using std::vector;
template <typename T,
          typename It = typename std::remove_reference<decltype(begin(std::declval<vector<vector<T>>>()))>::type>
void mean_impl(It frow, It lrow, T &res)
{
    T sum = *(frow->begin());

    for (auto row_it = frow; row_it != lrow; ++row_it)
        for (auto col_it = row_it->begin(), col_end = row_it->end(); col_it != col_end; ++col_it)
            T::sum(*col_it, sum, sum);

    res = sum;
    return;
}

template <typename T> T mean(const vector<vector<T>> &vv)
{
    assert(vv.size() > 0);
    assert(vv[0].size() > 0);

    const size_t MIN_THREAD_WORK = 100000;

    size_t rows = vv.size();
    size_t cols = vv[0].size();

    size_t rows_per_thread = std::max(MIN_THREAD_WORK / cols, 1ul);
    size_t req_threads = std::max(rows / rows_per_thread, 1ul);

    size_t nthreads = determine_threads(req_threads);
    vector<std::thread> threads(nthreads);
    vector<T> results(nthreads + 1, vv[0][0]);

    size_t bsize = rows / nthreads;

    /////////////////// SPAWN ///////////////////

    size_t tidx = 0;
    size_t last_row = 0;
    for (; rows >= bsize * (tidx + 1); last_row += bsize, tidx += 1)
    {
        auto st = vv.begin() + static_cast<long int>(last_row);
        auto en = vv.begin() + static_cast<long int>(last_row + bsize);
        threads[tidx] = std::thread(mean_impl<T>, st, en, std::ref(results[tidx]));
    }
    auto remainder = rows - bsize * tidx;

    if (remainder > 0)
    {
        assert(tidx == nthreads);
        auto st = vv.begin() + static_cast<long int>(last_row);
        auto en = vv.end();
        mean_impl<T>(st, en, std::ref(results[tidx]));
        tidx++;
    }

    for (auto &&t : threads)
        t.join();

    T sum = vv[0][0];
    for (size_t i = 0; i < tidx; ++i)
        T::sum(results[i], sum, sum);

    return sum;
}


static bool check_ext(const std::string & s, const std::vector<std::string>& exts){
    for (auto ext : exts)
            if (s.substr(s.find_last_of(".") + 1) == ext)
                return true;
        return false;
}


} // namespace clib