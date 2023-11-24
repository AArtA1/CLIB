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

// static bool check_ext(const std::string &s, const std::vector<std::string> &exts)
// {
//     for (auto ext : exts)
//         if (s.substr(s.find_last_of(".") + 1) == ext)
//             return true;
//     return false;
// }

/*!
 * \brief Обёртка над Image
 *
 * \details Класс содержит в себе двумерный массив с числами типа T и реализует функции для работы с массивом
 */
using std::vector;
template <typename T> class img final
{
    vector<vector<T>> vv;

    size_t rows_ = 0;
    size_t cols_ = 0;

    T inv_rows_{};
    T inv_cols_{};

  public:
    /*! @brief Инициализации двумерного массива
     *
     * \param[in] prorotype Элемент, которым нужно заполнить массив
     * \param[in] rows Количество строк
     * \param[in] cols Количество столбцов
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, инициализация будет разделена на отпимальное количество потоков
     */
    img(const T &prototype, size_t rows, size_t cols, size_t req_threads = 0)
    {
        // Вычислен c помощью функции determine_work_number;
        const size_t MIN_THREAD_WORK = 12000;
        rows_ = rows;
        cols_ = cols;

        vv.reserve(rows_);
        vv.resize(rows_);

        size_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        // разбиваем по потокам
        work(nthreads, &img::init_impl, std::ref(prototype));

        set_inv_rows();
        set_inv_cols();
    }

    /*! @brief Подсчет средного двумерного массива
     *
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, подсчет среднего будет разделен на отпимальное количество потоков
     */
    T mean(size_t req_threads = 0)
    {
        // Вычислен c помощью функции determine_work_number;
        const size_t MIN_THREAD_WORK = 7000;

        size_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        vector<T> results(vv.size(), vv[0][0]);

        // Разбиваем по потокам
        work(nthreads, &img::mean_impl, std::ref(results));

        // Собираем промежуточные суммы с потоков
        T ans = vv[0][0];
        for (auto &&elem : results)
            T::sum(elem, ans, ans);
        T::mult(inv_rows(), ans, ans);

        return ans;
    }

    static void determine_work_number(const T& prototype)
    {
        struct test_res
        {
            size_t t_init;
            size_t t_calc;
            float mean;
        };
        auto calc = [&prototype](size_t req_threads, size_t rows, size_t cols)
        {
            auto s_begin = std::chrono::steady_clock::now();

            auto one = T::from_float(prototype, 1);
            img im(one, rows, cols, req_threads);

            auto s_init = std::chrono::steady_clock::now();
            auto mean = im.mean(req_threads);
            auto s_end = std::chrono::steady_clock::now();

            size_t t_init = std::chrono::duration_cast<std::chrono::microseconds>(s_init - s_begin).count();
            size_t t_calc = std::chrono::duration_cast<std::chrono::microseconds>(s_end - s_init).count();

            test_res tr = {t_init, t_calc, mean.to_float()};
            return tr;
        };

        const size_t tests_num = 33;
        const size_t means_num = 40;
        const size_t rows = 1920;
        const size_t cols = 1080;

        // Прогреваем кэши
        for (size_t i = 0; i < tests_num; i += 4)
            calc(i, rows, cols);

        std::vector<test_res> results;
        for (size_t i = 0; i < tests_num; ++i)
        {
            // усреднение
            test_res cur_res = {0, 0, 0};
            for (size_t j = 0; j < means_num; ++j)
            {
                test_res temp_res = calc(i, rows, cols);
                cur_res.t_init += temp_res.t_init;
                cur_res.t_calc += temp_res.t_calc;
                cur_res.mean = temp_res.mean;
            }
            results.push_back({cur_res.t_init / means_num, cur_res.t_calc / means_num, cur_res.mean});
        }


        for (size_t i = 0; i < tests_num; ++i)
        {
            std::cout << "for i = " << std::setw(2) <<  i;
            std::cout << " init = " << std::setw(5) << results[i].t_init;
            std::cout << " calc = " << std::setw(5) << results[i].t_calc;
            std::cout << " mean = " << results[i].mean << std::endl;
        }
    }

  private:
    // Работа для одного потока
    // [st_row, en_row] - диапазон работы
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

    // Работа для одного потока
    // [st_row, en_row] - диапазон работы
    // res - массив промежуточных сумм
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
        std::flush(std::cout);
    }

    template <typename Func, typename... Args> void work(size_t nthreads, Func func, Args... args)
    {
        size_t rows = vv.size();
        assert(rows != 0);

        vector<std::thread> threads(nthreads);
        size_t bsize = std::max(rows / nthreads, 1ul);

        /////////////////// Создаем потоки ///////////////////
        size_t tidx = 0;
        size_t last_row = 0;
        for (; rows >= bsize * (tidx + 1) && tidx < nthreads; last_row += bsize, tidx += 1)
        {
            assert(tidx < nthreads);
            auto st = last_row;
            auto en = last_row + bsize;
            threads[tidx] = std::thread(func, this, st, en, args...);
        }

        // Обрабатываем остаток работ
        auto remainder = rows - bsize * tidx;
        if (remainder > 0)
        {
            assert(tidx == nthreads);
            auto st = last_row;
            auto en = rows;
            std::invoke(func, this, st, en, args...);
        }
        for (size_t th = 0; th < tidx; ++th)
            threads[th].join();

        return;
    }

    // Определение оптимального количество потоков исходя из количества работы и параметров системы
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