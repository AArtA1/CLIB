#pragma once
#include "logs.hpp"
#include <algorithm>
#include <bitset>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ostream>
#include <sstream>
#include <string>
#include <thread>
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

using std::vector;
template <typename T> class img final
{
    vector<vector<T>> vv;

    size_t rows_ = 0;
    size_t cols_ = 0;

    T inv_rows_{};
    T inv_cols_{};

  public:
    img(const T &prototype, size_t rows, size_t cols, size_t req_threads = 0)
    {
        const size_t MIN_THREAD_WORK = 10000000;
        rows_ = rows;
        cols_ = cols;

        vv.reserve(rows_);
        vv.resize(rows_);

        size_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        auto threads = work(nthreads, &img::init_impl, std::ref(prototype));

        for (auto &&t : threads)
            t.join();

        set_inv_rows();
        set_inv_cols();
    }

    T mean(size_t req_threads = 0)
    {
        const size_t MIN_THREAD_WORK = 10000000;

        size_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        vector<T> results(vv.size(), vv[0][0]);

        auto threads = work(nthreads, &img::mean_impl, std::ref(results));
        for (auto &&t : threads)
            t.join();

        T ans = vv[0][0];
        for (auto &&elem : results)
            T::sum(elem, ans, ans);

        T::mult(inv_rows(), ans, ans);
        return ans;
    }

  private:
    void init_impl(size_t st_row, size_t en_row, const T &prototype)
    {
        assert(cols_ != 0);
        assert(en_row >= st_row);

        for (auto i = st_row; i < en_row; ++i)
        {
            vv[i].reserve(cols_);
            for (size_t j = 0; j < cols_; ++j)
                vv[i].push_back(prototype);
        }
    }

    void mean_impl(size_t st_row, size_t en_row, vector<T> &res)
    {
        assert(en_row >= st_row);

        for (auto i = st_row; i < en_row; ++i)
        {
            T sum = T::from_float(vv[i][0], 0);
            for (auto &&elem : vv[i])
                T::sum(elem, sum, sum);

            T::mult(inv_cols(), sum, sum);
            res[i] = sum;
        }
    }

    template <typename Func, typename... Args> vector<std::thread> work(size_t nthreads, Func func, Args... args)
    {
        size_t rows = vv.size();
        assert(rows != 0);

        vector<std::thread> threads(nthreads);
        size_t bsize = rows / nthreads;

        std::cout << "nthreads = " << nthreads << std::endl;
        std::cout << "bsize = " << bsize << std::endl;

        /////////////////// SPAWN ///////////////////

        size_t tidx = 0;
        size_t last_row = 0;
        for (; rows >= bsize * (tidx + 1); last_row += bsize, tidx += 1)
        {
            auto st = last_row;
            auto en = last_row + bsize;
            threads[tidx] = std::thread(func, this, st, en, args...);
        }
        auto remainder = rows - bsize * tidx;

        if (remainder > 0)
        {
            assert(tidx == nthreads);
            auto st = last_row;
            auto en = rows;
            std::invoke(func, this, st, en, args...);
            tidx++;
        }

        return threads;
    }

    size_t determine_threads(size_t min_thread_work)
    {
        assert(rows_ != 0);
        assert(cols_ != 0);

        size_t rows_per_thread = std::max(min_thread_work / cols_, 1ul);
        size_t det_threads = std::max(rows_ / rows_per_thread, 1ul);

        size_t hard_conc = static_cast<size_t>(std::thread::hardware_concurrency());
        return std::min(hard_conc != 0 ? hard_conc : 2, det_threads);
    }

    void set_inv_rows()
    {
        inv_rows_ = T::from_float(vv[0][0], rows_);
        T::inv(inv_rows_, inv_rows_);
    }

    void set_inv_cols()
    {
        inv_cols_ = T::from_float(vv[0][0], cols_);
        T::inv(inv_cols_, inv_cols_);
    }

    T inv_cols()
    {
        return inv_cols_;
    }

    T inv_rows()
    {
        return inv_rows_;
    }
};

} // namespace clib