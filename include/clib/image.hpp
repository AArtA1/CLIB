#pragma once

#include "Flexfloat.hpp"
#include "ImgView.hpp"
#include "common.hpp"
#include "logs.hpp"

#include <iostream>

#ifdef PYBIND
namespace py = pybind11;
#include "pybind11/numpy.h"
#endif

namespace clib
{

/*!
 * \brief Обёртка над одноцветным Image
 *
 * \details Класс содержит в себе двумерный массив с числами типа T и реализует функции для работы с массивом
 */
using std::vector;
using pixel_t = ImgView::pixel_t;
using idx_t = ImgView::idx_t;
template <typename T> class img final
{
    idx_t rows_ = 0; // height of image
    idx_t cols_ = 0; // width of image

  public:
    vector<vector<T>> vv_;

    /*! @brief Инициализации изображения одинаковыми значениями
     *
     * \param[in] prorotype Элемент, которым нужно заполнить массив
     * \param[in] rows Количество строк
     * \param[in] cols Количество столбцов
     */
    img(const T &prototype, idx_t rows, idx_t cols, idx_t req_threads = 0) : vv_()
    {
        auto get_val = [&prototype](idx_t, idx_t) { return prototype; };
        _ctor_implt(rows, cols, get_val, req_threads);
    }

    /*! @brief Инициализации изображения из Представления
     *
     * \param[in] prorotype Элемент, из которого берутся гиперпараметры
     * \param[in] view Представление изображения
     * \param[in] clr Номер цвета
     */
    img(const T &prototype, const ImgView &view, idx_t clr = 0, idx_t req_threads = 0) : vv_()
    {
        auto get_val = [&prototype, &view, clr](idx_t i, idx_t j) {
            return T::from_float(prototype, view.get(i, j, clr));
        };
        _ctor_implt(view.rows(), view.cols(), get_val, req_threads);
    }

    /*! @brief Инициализации изображения массивом
     *
     * \param[in] prorotype Элемент, из которого берутся гиперпараметры
     * \param[in] img_path Путь до чёрно-белого изображения
     */
    img(const vector<vector<T>> &base, idx_t req_threads = 0) : vv_()
    {
        assert(base.size() > 0);

        auto rows = static_cast<idx_t>(base.size());
        auto cols = static_cast<idx_t>(base[0].size());
        auto get_val = [&base](idx_t i, idx_t j) { return base[i][j]; };

        _ctor_implt(rows, cols, get_val, req_threads);
    }

#ifdef PYBIND
    img(const py::array &base) : vv_()
    {
        assert(base.ndim() == 2);
        assert(base.shape(0) > 0);
        assert(base.shape(1) > 0);
        assert(base.dtype().num() == 7); // int

        auto rows = base.shape(0);
        auto cols = base.shape(1);
        auto get_val = [&base](idx_t, idx_t) {
            float val = static_cast<float>(*(reinterpret_cast<const int *>(base.data(i, j))));
            vv_[i].push_back(T::from_float(8, 23, 127, val));
        };

        _ctor_implt(rows, cols, get_val, req_threads);
    }
#endif

    const vector<vector<T>> vv() const
    {
        return vv_;
    }
    idx_t rows() const
    {
        return static_cast<idx_t>(vv_.size());
    }
    idx_t cols() const
    {
        return vv_.empty() ? 0 : static_cast<idx_t>(vv_[0].size());
    }

    /*! @brief Подсчет суммы двумерного массива
     *
     * Суммирование - операция НЕ ассоциативная, поэтому необходимо определиться в порядке суммирования. Чтобы в clib и
     * vlib получился одинаковый результат. В общем случае суммировани одной строки происходит не последовательно, то
     * есть не нулевой пиксель + первый пиксель + второй пиксель + и т.д. Суммирование происходит последовательно для
     * одинаковых остатков по модулю modulus. Далее все промежуточные суммы складываются последовательно. Ниже приведен
     * пример для сложения строки длиною 9 символов, modulus = 3
     *
     *  0   1   2   3   4   5   6   7   8
     *  |           |           |
     *  |           |           |
     * (0     +     3)    +     6 ------------> part_sums[0]
     *      |           |           |
     *      |           |           |
     *     (1     +     4)    +     7 --------> part_sums[1]
     *           |          |           |
     *           |          |           |
     *          (2    +     5)    +     8 ----> part_sums[2]
     *
     * sum = part_sums[0] + part_sums[1] + part_sums[2]
     *
     */
    T sum(idx_t req_threads = 0) const
    {
        const idx_t modulus = 3;
        auto modulus_sum = [modulus](const vector<T> &line) {
            vector<T> part_sums(line.begin(), line.begin() + modulus);
            // for (idx_t j = 0; j < modulus; ++j)
            //     std::cout << "part_sums[" << j << "] " << part_sums[j] << " = " << part_sums[j].to_float() <<
            //     std::endl;

            for (idx_t j = modulus; j < line.size(); ++j)
            {
                T::sum(line[j], part_sums[j % modulus], part_sums[j % modulus]);
                // std::cout << "part_sums[" << j % modulus << "] " << part_sums[j % modulus] << " = " << part_sums[j %
                // modulus].to_float() << std::endl;
            }

            // std::cout << "part_sums done" << std::endl;

            // Собираем промежуточные суммы для разных остатков по модулю
            auto st_indx = (line.size() - modulus) % modulus;
            T ans = part_sums[st_indx];
            for (idx_t j = 1; j < modulus; ++j)
                T::sum(part_sums[(st_indx + j) % modulus], ans, ans);

            return ans;
        };

        // Вычислен c помощью функции determine_work_number;
        const idx_t MIN_THREAD_WORK = 12000;
        idx_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        vector<T> results(rows_, vv_[0][0]);
        work(nthreads, rows_, [&](idx_t st_row, idx_t en_row) {
            for (idx_t i = st_row; i < en_row; ++i)
            {
                results[i] = modulus_sum(vv_[i]);
                // std::cout << "line[" << i << "]  " << results[i]  << " = " << results[i].to_float() << std::endl;
            }
        });

        // Собираем промежуточные суммы с потоков
        return modulus_sum(results);
    }

    /// @brief Подсчет среднего двумерного массива
    T mean() const
    {
        T summ = sum();
        T volume = T::from_float(summ, static_cast<float>(rows_ * cols_));
        T inv_volume = T::from_float(summ, 0);
        T::inv(volume, inv_volume);

#ifndef NDEBUG
        CLOG(debug) << "Mean:" << summ << " Float:" << summ.to_float();
        CLOG(debug) << "Volume:" << (rows_ * cols_);
        CLOG(debug) << "Volume value: " << volume;
        CLOG(debug) << "Volume inv_value: " << inv_volume;
#endif

        T::mult(summ, inv_volume, summ);
        return summ;
    }

    /// @brief Подсчет максимального элемента для каждой ячейки нескольких массивов
    template <typename... Imgs> static img max(const img &first, const Imgs &...imgs)
    {
        img res(first);
        for_each(first.rows(), first.cols(), [&](idx_t i, idx_t j) {
            res(i, j) = std::max({first(i, j), imgs(i, j)...});
        });

        return res;
    }

    /// @brief Обрезает все числа в двумерном массиве между minn и maxx
    img clip(pixel_t minn = 0, pixel_t maxx = 255) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) {
            if (vv_[i][j].to_float() < minn)
                res.vv_[i][j] = T::from_float(vv_[i][j], minn);
            else if (vv_[i][j].to_float() > maxx)
                res.vv_[i][j] = T::from_float(vv_[i][j], maxx);
            else
                res.vv_[i][j] = vv_[i][j];
        });

        return res;
    }

    img operator+(const T &rhs) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) { T::sum(res.vv_[i][j], rhs, res.vv_[i][j]); });

        return res;
    }
    img operator*(const T &rhs) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) { T::mult(res.vv_[i][j], rhs, res.vv_[i][j]); });

        return res;
    }
    img operator-(const T &rhs) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) { T::sub(res.vv_[i][j], rhs, res.vv_[i][j]); });

        return res;
    }
    img operator/(const T &rhs) const
    {
        img res(*this);
        T inv_rhs(rhs);
        T::inv(rhs, inv_rhs);
        const auto copy_inv = inv_rhs;

        for_each(rows_, cols_, [&](idx_t i, idx_t j) { 
            T::mult(res.vv_[i][j], copy_inv, res.vv_[i][j]); 
        });
        return res;
    }

    template <typename U> friend img<U> operator+(const U &lhs, const img<U> &rhs);
    template <typename U> friend img<U> operator*(const U &lhs, const img<U> &rhs);
    template <typename U> friend img<U> operator-(const U &lhs, const img<U> &rhs);

    img operator+(const img &rhs) const
    {
        assert(cols_ == rhs.cols_);
        assert(rows_ == rhs.rows_);

        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) { T::sum(res.vv_[i][j], rhs.vv_[i][j], res.vv_[i][j]); });

        return res;
    }
    img operator*(const img &rhs) const
    {
        assert(cols_ == rhs.cols_);
        assert(rows_ == rhs.rows_);

        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) { T::mult(res.vv_[i][j], rhs.vv_[i][j], res.vv_[i][j]); });

        return res;
    }
    img operator-(const img &rhs) const
    {
        assert(cols_ == rhs.cols_);
        assert(rows_ == rhs.rows_);

        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) { T::sub(res.vv_[i][j], rhs.vv_[i][j], res.vv_[i][j]); });

        return res;
    }
    img operator/(const img &rhs) const
    {
        assert(cols_ == rhs.cols_);
        assert(rows_ == rhs.rows_);

        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) {
            T inverted(res.vv_[i][j]);
            T::inv(rhs.vv_[i][j], inverted);
            T::mult(res.vv_[i][j], inverted, res.vv_[i][j]);
        });

        return res;
    }

    T &operator()(const idx_t &i, const idx_t &j)
    {
        assert(i < vv_.size() && j < vv_[0].size());
        return vv_[i][j];
    }
    const T &operator()(const idx_t &i, const idx_t &j) const
    {
        assert(i < vv_.size() && j < vv_[0].size());
        return vv_[i][j];
    }

    /*! @brief Записывает одноцветынй кадр в Представление
     *
     * \param[in] view Представление изображение
     * \param[in] clr Номер цвета
     */
    void write(ImgView &view, idx_t clr = 0)
    {
        for (idx_t i = 0; i < rows_; ++i)
            for (idx_t j = 0; j < cols_; ++j)
                view.set(vv_[i][j].to_float(), i, j, clr);
        
    }

  private:
    template <typename Func> void _ctor_implt(idx_t rows, idx_t cols, Func get_val, idx_t req_threads = 0)
    {
        assert(cols > 0);
        assert(rows > 0);

        // Вычислен c помощью функции determine_work_number;
        const idx_t MIN_THREAD_WORK = 12000;
        rows_ = rows;
        cols_ = cols;

        vv_.reserve(rows_);
        vv_.resize(rows_);

        idx_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        // разбиваем по потокам
        work(nthreads, rows_, [&](idx_t st_row, idx_t en_row) {
            for (auto i = st_row; i < en_row; ++i)
            {
                vv_[i].reserve(cols_);
                for (idx_t j = 0; j < cols_; ++j)
                    vv_[i].push_back(get_val(i, j));
            }
        });
    }

    // Выполняет func для каждого элемента в матрице, размерами rows и cols. Работа разделяется по потокам
    // Пример использования в operator+
    template <typename Func> static void for_each(idx_t rows, idx_t cols, Func func)
    {
        assert(rows != 0);
        assert(cols != 0);

        auto do_func = [&](idx_t st, idx_t en) {
            for (idx_t i = st; i < en; ++i)
                for (idx_t j = 0; j < cols; ++j)
                    func(i, j);
        };

        const idx_t MIN_THREAD_WORK = 10000;
        idx_t nthreads = determine_threads(rows, cols, MIN_THREAD_WORK);

        work(nthreads, rows, do_func);
    }

    // Выполняет func над this, разделяя работу на nthreads потоков
    // Пример использования в mean
    template <typename Func, typename... Args> static void work(idx_t nthreads, idx_t rows, Func func, Args... args)
    {
        assert(nthreads > 0);
        assert(rows != 0);

        if (nthreads == 1)
        {
            std::invoke(func, 0, rows, args...);
            return;
        }

        vector<std::thread> threads(nthreads);
        idx_t bsize = std::max(rows / nthreads, 1u);

        /////////////////// Создаем потоки ///////////////////
        idx_t tidx = 0;
        idx_t last_row = 0;
        for (; rows >= bsize * (tidx + 1) && tidx < nthreads; last_row += bsize, tidx += 1)
            threads[tidx] = std::thread(func, last_row, last_row + bsize, args...);

        // Обрабатываем остаток работ
        auto remainder = rows - bsize * tidx;
        if (remainder > 0)
            std::invoke(func, last_row, rows, args...);

        // Ждем потоки
        for (idx_t th = 0; th < tidx; ++th)
            threads[th].join();
    }

    // Определение оптимального количество потоков исходя из количества работы и параметров системы
    static idx_t determine_threads(idx_t rows, idx_t cols, idx_t min_thread_work)
    {
        assert(rows != 0);
        assert(cols != 0);

        idx_t rows_per_thread = std::max(min_thread_work / cols, 1u);
        idx_t det_threads = std::max(rows / rows_per_thread, 1u);

        idx_t hard_conc = static_cast<idx_t>(std::thread::hardware_concurrency());
        return std::min(hard_conc != 0 ? hard_conc : 2, det_threads);
    }
    idx_t determine_threads(idx_t min_thread_work) const
    {
        return determine_threads(rows_, cols_, min_thread_work);
    }

    // Вспомогательная функция для определения оптимального количество потоков для параллелизма
    static void determine_work_number(const T &prototype)
    {
        struct test_res
        {
            long int t_init;
            long int t_calc;
            float mean;
        };
        auto calc = [&prototype](idx_t req_threads, idx_t rows, idx_t cols) {
            auto s_begin = std::chrono::steady_clock::now();

            auto one = T::from_float(prototype, 1);
            img im(one, rows, cols, req_threads);

            auto s_init = std::chrono::steady_clock::now();
            auto mean = im.mean();
            auto s_end = std::chrono::steady_clock::now();

            auto t_init = std::chrono::duration_cast<std::chrono::microseconds>(s_init - s_begin).count();
            auto t_calc = std::chrono::duration_cast<std::chrono::microseconds>(s_end - s_init).count();

            test_res tr = {t_init, t_calc, mean.to_float()};
            return tr;
        };

        const idx_t tests_num = 33;
        const idx_t means_num = 40;
        const idx_t rows = 500;
        const idx_t cols = 500;

        // Прогреваем кэши
        for (idx_t i = 0; i < tests_num; i += 4)
            calc(i, rows, cols);

        std::vector<test_res> results;
        for (idx_t i = 0; i < tests_num; ++i)
        {
            // усреднение
            test_res cur_res = {0, 0, 0};
            for (idx_t j = 0; j < means_num; ++j)
            {
                test_res temp_res = calc(i, rows, cols);
                cur_res.t_init += temp_res.t_init;
                cur_res.t_calc += temp_res.t_calc;
                cur_res.mean = temp_res.mean;
            }
            results.push_back({cur_res.t_init / means_num, cur_res.t_calc / means_num, cur_res.mean});
        }
    }
};

template <typename T> img<T> operator+(const T &lhs, const img<T> &rhs)
{
    img<T> res(rhs);
    img<T>::for_each(rhs.rows(), rhs.cols(), [&](idx_t i, idx_t j) { T::sum(lhs, res.vv_[i][j], res.vv_[i][j]); });

    return res;
}
template <typename T> img<T> operator*(const T &lhs, const img<T> &rhs)
{
    img<T> res(rhs);
    img<T>::for_each(rhs.rows(), rhs.cols(), [&](idx_t i, idx_t j) { T::mult(lhs, res.vv_[i][j], res.vv_[i][j]); });

    return res;
}
template <typename T> img<T> operator-(const T &lhs, const img<T> &rhs)
{
    img<T> res(rhs);
    img<T>::for_each(rhs.rows(), rhs.cols(), [&](idx_t i, idx_t j) { T::sub(lhs, res.vv_[i][j], res.vv_[i][j]); });

    return res;
}

extern template class img<Flexfloat>;

/*!
 * \brief Обёртка над трёхцветным Image
 *
 * \details Класс содержит в себе три двумерных массива с числами типа T и реализует функции для работы с ними
 */
template <typename T> class img_rgb
{
    img<T> r_, g_, b_;

  public:
    /*! @brief Поканальная инициализации изображения
     *
     * \param[in] r Интенсивности красного
     * \param[in] g Интенсивности зелёного
     * \param[in] b Интенсивности голубого
     */
    img_rgb(img<T> r, img<T> g, img<T> b) : r_(r.clip()), g_(g.clip()), b_(b.clip())
    {
    }

    /*! @brief Инициализации изображения из Представления
     *
     * \param[in] prorotype Элемент, из которого берутся гиперпараметры
     * \param[in] view Представление изображения
     */
    img_rgb(const T &prototype, const ImgView &view, idx_t req_threads = 0)
        : r_(prototype, view, ImgView::R, req_threads), 
          g_(prototype, view, ImgView::G, req_threads),
          b_(prototype, view, ImgView::B, req_threads)
    {
        assert(view.clrs() == 3);
    }

    /*! @brief Записывает трёхцветный кадр в Представление
     *
     * \param[in] view Представление изображения
     */
    void write(ImgView &view)
    {
        r_.write(view, ImgView::R);
        g_.write(view, ImgView::G);
        b_.write(view, ImgView::B);
    }

    // rows - height of image
    idx_t rows() const
    {
        return r_.rows();
    }
    // cols - width of image
    idx_t cols() const
    {
        return r_.cols();
    }
    const img<T> &r() const
    {
        return r_;
    }
    const img<T> &g() const
    {
        return g_;
    }
    const img<T> &b() const
    {
        return b_;
    }

    img_rgb operator+(const T &rhs) const
    {
        img_rgb<T> res;

        res.r_ = r_ + rhs;
        res.g_ = g_ + rhs;
        res.b_ = b_ + rhs;

        return res;
    }
    img_rgb operator*(const T &rhs) const
    {
        img_rgb<T> res;

        res.r_ = r_ * rhs;
        res.g_ = g_ * rhs;
        res.b_ = b_ * rhs;

        return res;
    }
    img_rgb operator-(const T &rhs) const
    {
        img_rgb<T> res;

        res.r_ = r_ - rhs;
        res.g_ = g_ - rhs;
        res.b_ = b_ - rhs;

        return res;
    }

    template <typename U> friend img_rgb<U> operator+(const U &lhs, const img_rgb<U> &rhs);
    template <typename U> friend img_rgb<U> operator*(const U &lhs, const img_rgb<U> &rhs);
    template <typename U> friend img_rgb<U> operator-(const U &lhs, const img_rgb<U> &rhs);

    img_rgb operator+(const img_rgb &rhs) const
    {
        assert(cols() == rhs.cols());
        assert(rows() == rhs.rows());

        img_rgb res;

        res.r_ = r_ + rhs.r;
        res.g_ = g_ + rhs.g;
        res.b_ = b_ + rhs.b;

        return res;
    }
    img_rgb operator*(const img_rgb &rhs) const
    {
        assert(cols() == rhs.cols());
        assert(rows() == rhs.rows());

        img_rgb res;

        res.r_ = r_ * rhs.r;
        res.g_ = g_ * rhs.g;
        res.b_ = b_ * rhs.b;

        return res;
    }
    img_rgb operator-(const img_rgb &rhs) const
    {
        assert(cols() == rhs.cols());
        assert(rows() == rhs.rows());

        img_rgb res;

        res.r_ = r_ - rhs.r;
        res.g_ = g_ - rhs.g;
        res.b_ = b_ - rhs.b;

        return res;
    }
};

template <typename T> img_rgb<T> operator+(const T &lhs, const img_rgb<T> &rhs)
{
    img_rgb<T> res;

    res.r_ = lhs + rhs.r;
    res.g_ = lhs + rhs.g;
    res.b_ = lhs + rhs.b;

    return res;
}
template <typename T> img_rgb<T> operator*(const T &lhs, const img_rgb<T> &rhs)
{
    img_rgb<T> res;

    res.r_ = lhs * rhs.r;
    res.g_ = lhs * rhs.g;
    res.b_ = lhs * rhs.b;

    return res;
}
template <typename T> img_rgb<T> operator-(const T &lhs, const img_rgb<T> &rhs)
{
    img_rgb<T> res;

    res.r_ = lhs - rhs.r;
    res.g_ = lhs - rhs.g;
    res.b_ = lhs - rhs.b;

    return res;
}

} // namespace clib
