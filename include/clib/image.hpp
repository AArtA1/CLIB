#pragma once

#define cimg_use_png
#define cimg_use_jpeg
#include "CImg.h"
#include "X11/Xlib.h"
#include "common.hpp"
#include <iostream>

namespace clib
{

static bool check_ext(const std::string &s, const std::vector<std::string> &exts)
{
    for (auto ext : exts)
        if (s.substr(s.find_last_of(".") + 1) == ext)
            return true;
    return false;
}

template <typename T> cimg_library::CImg<T> read_img(const std::string &path)
{
    cimg_library::CImg<T> image;
    if (check_ext(path, {
                            "jpeg",
                            "jpg",
                        }))
    {

        image = cimg_library::CImg<T>::get_load_jpeg(path.c_str());
    }
    else if (check_ext(path, {
                                 "png",
                             }))
    {
        image = cimg_library::CImg<T>::get_load_png(path.c_str());
    }
    else
    {
#ifndef NDEBUG
        CLOG(error) << "Unknown format: " << path.substr(path.find_last_of(".") + 1) << std::endl;
#endif
        throw "Unknown format. Please check again";
    }
    return image;
}

template <typename T> void write_img(const cimg_library::CImg<T> &image, const std::string &path)
{
    if (check_ext(path, {
                            "jpeg",
                            "jpg",
                        }))
    {
        image.save_jpeg(path.c_str());
    }
    else if (check_ext(path, {
                                 "png",
                             }))
    {
        image.save_png(path.c_str());
    }
    else
    {
#ifndef NDEBUG
        CLOG(error) << "Unknown format: " << path.substr(path.find_last_of(".") + 1) << std::endl;
#endif
        throw "Unknown format. Please check again";
    }
}

/*!
 * \brief Обёртка над Image
 *
 * \details Класс содержит в себе двумерный массив с числами типа T и реализует функции для работы с массивом
 */
using std::vector;
using IMG_T = float;
template <typename T> class img final
{
    size_t rows_ = 0;
    size_t cols_ = 0;

    T inv_rows_{};
    T inv_cols_{};

  public:
    vector<vector<T>> vv_;

    /*! @brief Инициализации двумерного массива одинаковыми значениями
     *
     * \param[in] prorotype Элемент, которым нужно заполнить массив
     * \param[in] rows Количество строк
     * \param[in] cols Количество столбцов
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, инициализация будет разделена на отпимальное количество потоков
     */
    img(const T &prototype, size_t rows, size_t cols, size_t req_threads = 0)
    {
        assert(cols_ != 0);
        assert(rows_ != 0);

        // Вычислен c помощью функции determine_work_number;
        const size_t MIN_THREAD_WORK = 12000;
        rows_ = rows;
        cols_ = cols;

        vv_.reserve(rows_);
        vv_.resize(rows_);

        size_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        // разбиваем по потокам
        work(nthreads, [&](size_t st_row, size_t en_row) {
            for (auto i = st_row; i < en_row; ++i)
            {
                vv_[i].reserve(cols_);
                for (size_t j = 0; j < cols_; ++j)
                    vv_[i].push_back(prototype);
            }
        });

        set_inv_rows();
        set_inv_cols();
    }

    /*! @brief Инициализации двумерного массива чёрно-белым изображением
     *
     * \param[in] prorotype Элемент, из которого берутся гиперпараметры
     * \param[in] img_path Путь до чёрно-белого изображения
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, инициализация будет разделена на отпимальное количество потоков
     */
    img(const T &prototype, const std::string &img_path, size_t req_threads = 0)
    {
        // Вычислен c помощью функции determine_work_number TODO
        const size_t MIN_THREAD_WORK = 12000;

        cimg_library::CImg img_flt = read_img<IMG_T>(img_path);

        rows_ = img_flt.height();
        cols_ = img_flt.width();

        vv_.reserve(rows_);
        vv_.resize(rows_);

        size_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        // разбиваем по потокам
        work(nthreads, [&](size_t st_row, size_t en_row) {
            for (auto i = st_row; i < en_row; ++i)
            {
                vv_[i].reserve(cols_);
                for (size_t j = 0; j < cols_; ++j)
                    vv_[i].push_back(T::from_float(prototype, img_flt(j, i)));
            }
        });

        set_inv_rows();
        set_inv_cols();
    }

    /*! @brief Инициализации двумерного массива другим массивом
     *
     * \param[in] prorotype Элемент, из которого берутся гиперпараметры
     * \param[in] img_path Путь до чёрно-белого изображения
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, инициализация будет разделена на отпимальное количество потоков
     */
    img(const vector<vector<T>> &base, size_t req_threads = 0)
    {
        assert(base.size() > 0);
        assert(base[0].size() > 0);

        // Вычислен c помощью функции determine_work_number TODO
        const size_t MIN_THREAD_WORK = 12000;

        rows_ = base.size();
        cols_ = base[0].size();

        vv_.reserve(rows_);
        vv_.resize(rows_);

        size_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        // разбиваем по потокам
        work(nthreads, [&](size_t st_row, size_t en_row) {
            for (auto i = st_row; i < en_row; ++i)
            {
                vv_[i].reserve(cols_);
                for (size_t j = 0; j < cols_; ++j)
                    vv_[i].push_back(base[i][j]);
            }
        });

        set_inv_rows();
        set_inv_cols();
    }

    /*! @brief Записывает чёрно-белое изображение в файл
     *
     * \param[in] out_path  Выходной файл
     */
    void write(const std::string &out_path)
    {
        cimg_library::CImg<IMG_T> img_flt(cols_, rows_);

        for (size_t i = 0; i < rows_; ++i)
            for (size_t j = 0; j < cols_; ++j)
                img_flt(j, i) = vv_[i][j].to_float();

        clib::write_img(img_flt, out_path);
    }

    /*! @brief Подсчет суммы двумерного массива
     *
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, подсчет среднего будет разделен на отпимальное количество потоков
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
    T sum(size_t req_threads = 0) const
    {
        vector<T> results(rows_, vv_[0][0]);
        const size_t modulus = 3;

        // Вычислен c помощью функции determine_work_number;
        const size_t MIN_THREAD_WORK = 12000;
        size_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        work(nthreads, [&](size_t st_row, size_t en_row) {
            for (size_t i = st_row; i < en_row; ++i)
            {
                // Сумма по строке
                vector<T> part_sums(vv_[i].begin(), vv_[i].begin() + modulus);
                for (size_t j = modulus; j < cols_; ++j)
                    T::sum(vv_[i][j], part_sums[j % modulus], part_sums[j % modulus]);

                // Собираем промежуточные суммы для разных остатков по модулю
                T ans = part_sums[0];
                for (size_t j = 1; j < modulus; ++j)
                    T::sum(part_sums[j], ans, ans);

                results[i] = ans;
            }
        });

        // Собираем промежуточные суммы с потоков
        T ans = results[0];
        for (size_t i = 1; i < rows_; ++i)
            T::sum(results[i], ans, ans);

        return ans;
    }

    /*! @brief Подсчет среднего двумерного массива
     *
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, подсчет среднего будет разделен на отпимальное количество потоков
    */
    T mean(size_t req_threads = 0) const
    {
        T summ = sum();
        T::mult(summ, T::from_float(summ, 1.0 / (rows_ * cols_)), summ);

        return summ;
    }

    /*! @brief Обрезает все числа в двумерном массиве между minn и maxx
     *
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, подсчет среднего будет разделен на отпимальное количество потоков
    */
    img clip(size_t minn = 0, size_t maxx = 255, size_t req_threads = 0) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](size_t i, size_t j) {
            if (vv_[i][j].to_float() < minn)
                res.vv_[i][j] = T::from_float(vv_[i][j], minn);
            else if (vv_[i][j].to_float() > maxx)
                res.vv_[i][j] = T::from_float(vv_[i][j], maxx);
            else res.vv_[i][j] = vv_[i][j];
        });

        return res;
    }

    const vector<vector<T>> vv() const
    {
        return vv_;
    }

    const size_t rows() const
    {
        return rows_;
    }

    const size_t cols() const
    {
        return cols_;
    }

    static void determine_work_number(const T &prototype)
    {
        struct test_res
        {
            size_t t_init;
            size_t t_calc;
            float mean;
        };
        auto calc = [&prototype](size_t req_threads, size_t rows, size_t cols) {
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
        const size_t rows = 500;
        const size_t cols = 500;

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
            std::cout << "for i = " << std::setw(2) << i;
            std::cout << " init = " << std::setw(5) << results[i].t_init;
            std::cout << " calc = " << std::setw(5) << results[i].t_calc;
            std::cout << " mean = " << results[i].mean << std::endl;
        }
    }

    img operator+(const T &rhs) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](size_t i, size_t j) { T::sum(res.vv_[i][j], rhs, res.vv_[i][j]); });

        return res;
    }
    img operator*(const T &rhs) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](size_t i, size_t j) { T::mult(res.vv_[i][j], rhs, res.vv_[i][j]); });

        return res;
    }
    img operator-(const T &rhs) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](size_t i, size_t j) { T::sub(res.vv_[i][j], rhs, res.vv_[i][j]); });

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
        for_each(rows_, cols_, [&](size_t i, size_t j) { T::sum(res.vv_[i][j], rhs.vv_[i][j], res.vv_[i][j]); });

        return res;
    }
    img operator*(const img &rhs) const
    {
        assert(cols_ == rhs.cols_);
        assert(rows_ == rhs.rows_);

        img res(*this);
        for_each(rows_, cols_, [&](size_t i, size_t j) { T::mult(res.vv_[i][j], rhs.vv_[i][j], res.vv_[i][j]); });

        return res;
    }
    img operator-(const img &rhs) const
    {
        assert(cols_ == rhs.cols_);
        assert(rows_ == rhs.rows_);

        img res(*this);
        for_each(rows_, cols_, [&](size_t i, size_t j) { T::sub(res.vv_[i][j], rhs.vv_[i][j], res.vv_[i][j]); });

        return res;
    }

  private:
    // Выполняет func для каждого элемента в матрице, размерами rows и cols. Работа разделяется по потокам
    // Пример использования в operator+
    template <typename Func, typename... Args> 
    static void for_each(size_t rows, size_t cols, Func func, Args... args)
    {
        assert(rows != 0);
        assert(cols != 0);

        auto do_func = [&](size_t st, size_t en) {
            for (size_t i = st; i < en; ++i)
                for (size_t j = 0; j < cols; ++j)
                    func(i, j, args...);
        };

        const size_t MIN_THREAD_WORK = 10000;
        size_t nthreads = determine_threads(rows, cols, MIN_THREAD_WORK);
        if (nthreads == 1)
        {
            do_func(0, rows);
            return;
        }

        vector<std::thread> threads(nthreads);
        size_t bsize = std::max(rows / nthreads, 1ul);

        // Создаем потоки
        size_t tidx = 0;
        size_t last_row = 0;
        for (; rows >= bsize * (tidx + 1) && tidx < nthreads; last_row += bsize, tidx += 1)
            threads[tidx] = std::thread(do_func, last_row, last_row + bsize);

        // Обрабатываем остаток работ
        auto remainder = rows - bsize * tidx;
        if (remainder > 0)
            do_func(last_row, rows);

        // Ждем потоки
        for (size_t th = 0; th < tidx; ++th)
            threads[th].join();

        return;
    }

    // Выполняет func над this, разделяя работу на nthreads потоков
    // Пример использования в mean
    template <typename Func, typename... Args> void work(size_t nthreads, Func func, Args... args) const
    {
        assert(nthreads > 0);
        assert(rows_ != 0);
        assert(cols_ != 0);

        if (nthreads == 1)
        {
            std::invoke(func, 0, rows_, args...);
            return;
        }

        vector<std::thread> threads(nthreads);
        size_t bsize = std::max(rows_ / nthreads, 1ul);

        /////////////////// Создаем потоки ///////////////////
        size_t tidx = 0;
        size_t last_row = 0;
        for (; rows_ >= bsize * (tidx + 1); last_row += bsize, tidx += 1)
        {
            // std::cout << "tidx = " << tidx << std::endl;
            // std::cout << "nthreads = " << nthreads << std::endl;
            // std::flush(std::cout);

            assert(tidx < nthreads);
            auto st = last_row;
            auto en = last_row + bsize;
            threads[tidx] = std::thread(func, st, en, args...);
        }

        // Обрабатываем остаток работ
        auto remainder = rows_ - bsize * tidx;
        if (remainder > 0)
        {
            assert(tidx == nthreads);
            auto st = last_row;
            auto en = rows_;
            std::invoke(func, st, en, args...);
        }
        for (size_t th = 0; th < tidx; ++th)
            threads[th].join();

        return;
    }

    // Определение оптимального количество потоков исходя из количества работы и параметров системы
    static size_t determine_threads(size_t rows, size_t cols, size_t min_thread_work)
    {
        assert(rows != 0);
        assert(cols != 0);

        size_t rows_per_thread = std::max(min_thread_work / cols, 1ul);
        size_t det_threads = std::max(rows / rows_per_thread, 1ul);

        size_t hard_conc = static_cast<size_t>(std::thread::hardware_concurrency());
        return std::min(hard_conc != 0 ? hard_conc : 2, det_threads);
    }
    size_t determine_threads(size_t min_thread_work) const
    {
        return determine_threads(rows_, cols_, min_thread_work);
    }

    void set_inv_rows()
    {
        inv_rows_ = T::from_float(vv_[0][0], rows_);
        T::inv(inv_rows_, inv_rows_);
    }

    void set_inv_cols()
    {
        inv_cols_ = T::from_float(vv_[0][0], cols_);
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

template <typename T> img<T> operator+(const T &lhs, const img<T> &rhs)
{
    img<T> res(rhs);
    img<T>::for_each(rhs.rows(), rhs.cols(), [&](size_t i, size_t j) { T::sum(lhs, res.vv_[i][j], res.vv_[i][j]); });

    return res;
}

template <typename T> img<T> operator*(const T &lhs, const img<T> &rhs)
{
    img<T> res(rhs);
    img<T>::for_each(rhs.rows(), rhs.cols(), [&](size_t i, size_t j) { T::mult(lhs, res.vv_[i][j], res.vv_[i][j]); });

    return res;
}

template <typename T> img<T> operator-(const T &lhs, const img<T> &rhs)
{
    img<T> res(rhs);
    img<T>::for_each(rhs.rows(), rhs.cols(), [&](size_t i, size_t j) { T::sub(lhs, res.vv_[i][j], res.vv_[i][j]); });

    return res;
}

} // namespace clib
