#pragma once

#define cimg_use_png
#define cimg_use_jpeg

#include "CImg.h"
#include "Flexfloat.hpp"
#include "X11/Xlib.h"
#include "common.hpp"
#include "logs.hpp"

#include <iostream>

#ifdef PYBIND
namespace py = pybind11;
#include "pybind11/numpy.h"
#endif

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
        throw std::invalid_argument("Unknown format. Please check again");
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
        throw std::invalid_argument("Unknown format. Please check again");
    }
}

/*!
 * \brief Обёртка над Image
 *
 * \details Класс содержит в себе двумерный массив с числами типа T и реализует функции для работы с массивом
 */
using std::vector;
using img_t = float;
using idx_t = unsigned;
template <typename T> class img final
{
    idx_t rows_ = 0; // height of image
    idx_t cols_ = 0; // width of image
    // CImg stores data as [width,height]. Therefore, the data in cimg are transposed.

    T inv_rows_{};
    T inv_cols_{};

  public:
    vector<vector<T>> vv_;

    img() = default;

    /*! @brief Инициализации двумерного массива одинаковыми значениями
     *
     * \param[in] prorotype Элемент, которым нужно заполнить массив
     * \param[in] rows Количество строк
     * \param[in] cols Количество столбцов
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, инициализация будет разделена на отпимальное количество потоков
     */

    img(const T &prototype, idx_t rows, idx_t cols, idx_t req_threads = 0) : vv_()
    {
        auto get_val = [&prototype](idx_t, idx_t) { return prototype; };
        _ctor_implt(rows, cols, get_val, req_threads);
    }

    /*! @brief Инициализации двумерного массива чёрно-белым изображением
     *
     * \param[in] prorotype Элемент, из которого берутся гиперпараметры
     * \param[in] img_path Путь до чёрно-белого изображения
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, инициализация будет разделена на отпимальное количество потоков
     */
    img(const T &prototype, const std::string &img_path, idx_t req_threads = 0) : vv_()
    {
        cimg_library::CImg img_flt = read_img<img_t>(img_path);

        assert(img_flt.height() >= 0);
        assert(img_flt.width() >= 0);
        auto rows = static_cast<idx_t>(img_flt.height());
        auto cols = static_cast<idx_t>(img_flt.width());
        auto get_val = [&prototype, &img_flt](idx_t i, idx_t j) { return T::from_float(prototype, img_flt(j, i)); };

        _ctor_implt(rows, cols, get_val, req_threads);
    }

    img(const T &prototype, const cimg_library::CImg<img_t> &img_flt, idx_t req_threads = 0, idx_t depth = 0,
        idx_t dim = 0)
        : vv_()
    {
        assert(img_flt.height() >= 0);
        assert(img_flt.width() >= 0);
        auto rows = static_cast<idx_t>(img_flt.height());
        auto cols = static_cast<idx_t>(img_flt.width());
        auto get_val = [&prototype, &img_flt, &depth, &dim](idx_t i, idx_t j) {
            return T::from_float(prototype, img_flt(j, i, depth, dim));
        };

        _ctor_implt(rows, cols, get_val, req_threads);
    }

    /*! @brief Инициализации двумерного массива другим массивом
     *
     * \param[in] prorotype Элемент, из которого берутся гиперпараметры
     * \param[in] img_path Путь до чёрно-белого изображения
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, инициализация будет разделена на отпимальное количество потоков
     */
    img(const vector<vector<T>> &base, idx_t req_threads = 0) : vv_()
    {
        auto rows = static_cast<idx_t>(base.size());
        assert(base.size() > 0);
        auto cols = static_cast<idx_t>(base[0].size());
        auto get_val = [&base](idx_t i, idx_t j) { return base[i][j]; };

        _ctor_implt(rows, cols, get_val, req_threads);
    }

#ifdef PYBIND
    img(const py::array &base) : vv_()
    {
        // std::cout << base.ndim() << std::endl;
        // std::cout << base.shape(0) << std::endl;
        // std::cout << base.shape(1) << std::endl;
        // std::cout << base.dtype().num() << std::endl;

        assert(base.ndim() == 2);
        assert(base.shape(0) > 0);
        assert(base.shape(1) > 0);
        assert(base.dtype().num() == 7); // int

        // Вычислен c помощью функции determine_work_number TODO
        const idx_t MIN_THREAD_WORK = 12000;

        rows_ = base.shape(0);
        cols_ = base.shape(1);

        vv_.reserve(rows_);
        vv_.resize(rows_);

        idx_t nthreads = determine_threads(MIN_THREAD_WORK);

        // разбиваем по потокам
        work(nthreads, [&](idx_t st_row, idx_t en_row) {
            for (auto i = st_row; i < en_row; ++i)
            {
                vv_[i].reserve(cols_);
                for (idx_t j = 0; j < cols_; ++j)
                {
                    float val = static_cast<float>(*(reinterpret_cast<const int *>(base.data(i, j))));
                    vv_[i].push_back(T::from_float(8, 23, 127, val));
                }
            }
        });

        set_inv_rows();
        set_inv_cols();
    }
#endif

    /*! @brief Записывает чёрно-белое изображение в файл
     *
     * \param[in] out_path  Выходной файл
     */
    void write(const std::string &out_path)
    {
        cimg_library::CImg<img_t> img_flt(cols_, rows_);

        for (idx_t i = 0; i < rows_; ++i)
            for (idx_t j = 0; j < cols_; ++j)
                img_flt(j, i) = vv_[i][j].to_float();

        clib::write_img(img_flt, out_path);
    }

    void write(cimg_library::CImg<img_t> &img_flt, idx_t depth = 0, idx_t dim = 0)
    {
        for (idx_t i = 0; i < rows_; ++i)
            for (idx_t j = 0; j < cols_; ++j)
                img_flt(j, i, depth, dim) = vv_[i][j].to_float();
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
        work(nthreads, [&](idx_t st_row, idx_t en_row) {
            for (idx_t i = st_row; i < en_row; ++i)
            {
                results[i] = modulus_sum(vv_[i]);
                // std::cout << "line[" << i << "]  " << results[i]  << " = " << results[i].to_float() << std::endl;
            }
        });

        // Собираем промежуточные суммы с потоков
        return modulus_sum(results);
    }

    /*! @brief Подсчет среднего двумерного массива
     *
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, подсчет среднего будет разделен на отпимальное количество потоков
     */
    T mean() const
    {
        T summ = sum();

#ifndef NDEBUG
        CLOG(debug) << "Mean:" << summ << " Float:" << summ.to_float() << std::endl
                    << " Volume:" << (rows_ * cols_) << std::endl;
#endif

        T volume = T::from_float(summ, static_cast<float>(rows_ * cols_));

#ifndef NDEBUG
        CLOG(debug) << "Volume value: " << volume << std::endl;
#endif

        T inv_volume = T::from_float(summ, 0);

        T::inv(volume, inv_volume);

#ifndef NDEBUG
        CLOG(debug) << "Volume inv_value: " << inv_volume << std::endl;
#endif

        T::mult(summ, inv_volume, summ);

        return summ;
    }

    /*! @brief Обрезает все числа в двумерном массиве между minn и maxx
     *
     * \param[in] req_threads Количество потоков на которые необходимо разделить инициализацию. Если этот параметр не
     *                        задан, подсчет среднего будет разделен на отпимальное количество потоков
     */
    img clip(idx_t minn = 0, idx_t maxx = 255) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) {
            if (vv_[i][j].to_float() < static_cast<float>(minn))
                res.vv_[i][j] = T::from_float(vv_[i][j], static_cast<float>(minn));
            else if (vv_[i][j].to_float() > static_cast<float>(maxx))
                res.vv_[i][j] = T::from_float(vv_[i][j], static_cast<float>(maxx));
            else
                res.vv_[i][j] = vv_[i][j];
        });

        return res;
    }

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
        for_each(rows_, cols_, [&](idx_t i, idx_t j) { T::mult(res.vv_[i][j], inv_rhs, res.vv_[i][j]); });
        return res;
    }

    template <typename U> friend img<U> operator+(const U &lhs, const img<U> &rhs);
    template <typename U> friend img<U> operator*(const U &lhs, const img<U> &rhs);
    template <typename U> friend img<U> operator-(const U &lhs, const img<U> &rhs);
    // template <typename U> friend img<U> operator/(const U &lhs, const img<U> &rhs);

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
        T inverted(res.vv_[0][0]);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) {
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

    template <typename... IMGS> static img maxxxx(const img &first, const IMGS &...imgs)
    {
        img res(first);
        for_each(first.rows(), first.cols(), [&](idx_t i, idx_t j) {
            res(i, j) = std::max({first(i, j), imgs(i, j)...});
        });

        return res;
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
        work(nthreads, [&](idx_t st_row, idx_t en_row) {
            for (auto i = st_row; i < en_row; ++i)
            {
                vv_[i].reserve(cols_);
                for (idx_t j = 0; j < cols_; ++j)
                    vv_[i].push_back(get_val(i, j));
            }
        });

        set_inv_rows();
        set_inv_cols();
    }

    // Выполняет func для каждого элемента в матрице, размерами rows и cols. Работа разделяется по потокам
    // Пример использования в operator+
    template <typename Func, typename... Args> static void for_each(idx_t rows, idx_t cols, Func func, Args... args)
    {
        assert(rows != 0);
        assert(cols != 0);

        auto do_func = [&](idx_t st, idx_t en) {
            for (idx_t i = st; i < en; ++i)
                for (idx_t j = 0; j < cols; ++j)
                    func(i, j, args...);
        };

        const idx_t MIN_THREAD_WORK = 10000;
        idx_t nthreads = determine_threads(rows, cols, MIN_THREAD_WORK);
        if (nthreads == 1)
        {
            do_func(0, rows);
            return;
        }

        vector<std::thread> threads(nthreads);
        idx_t bsize = std::max(rows / nthreads, 1u);

        // Создаем потоки
        idx_t tidx = 0;
        idx_t last_row = 0;
        for (; rows >= bsize * (tidx + 1) && tidx < nthreads; last_row += bsize, tidx += 1)
            threads[tidx] = std::thread(do_func, last_row, last_row + bsize);

        // Обрабатываем остаток работ
        auto remainder = rows - bsize * tidx;
        if (remainder > 0)
            do_func(last_row, rows);

        // Ждем потоки
        for (idx_t th = 0; th < tidx; ++th)
            threads[th].join();

        return;
    }

    // Выполняет func над this, разделяя работу на nthreads потоков
    // Пример использования в mean
    template <typename Func, typename... Args> void work(idx_t nthreads, Func func, Args... args) const
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
        idx_t bsize = std::max(rows_ / nthreads, 1u);

        /////////////////// Создаем потоки ///////////////////
        idx_t tidx = 0;
        idx_t last_row = 0;
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
        for (idx_t th = 0; th < tidx; ++th)
            threads[th].join();

        return;
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

    void set_inv_rows()
    {
        inv_rows_ = T::from_float(vv_[0][0], static_cast<float>(rows_));
        T::inv(inv_rows_, inv_rows_);
    }

    void set_inv_cols()
    {
        inv_cols_ = T::from_float(vv_[0][0], static_cast<float>(cols_));
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
const int threads_quantity = 1;

//
template <typename T> class img_rgb
{
    const idx_t depth = 1; // depth
    img<T> r, g, b;        // spectrum
  public:
    enum spectrum
    {
        R,
        G,
        B
    };

    img_rgb() = default;

    img_rgb(img<T> r_, img<T> g_, img<T> b_) : r(r_.clip()), g(g_.clip()), b(b_.clip())
    {
    }

    img_rgb(const T &prototype, const std::string &img_path, idx_t req_threads = 0)
    {
        cimg_library::CImg img_flt = read_img<img_t>(img_path);

        assert(img_flt.spectrum() == 3);
        assert(img_flt.depth() == 1);

        r = img(prototype, img_flt, req_threads, depth - 1, R);
        g = img(prototype, img_flt, req_threads, depth - 1, G);
        b = img(prototype, img_flt, req_threads, depth - 1, B);
    }

    img_rgb(const T &prototype, const cimg_library::CImg<img_t> &img_flt, idx_t frame, idx_t req_threads = 0)
    {
        assert(img_flt.spectrum() == 3);

        r = img(prototype, img_flt, req_threads, frame, R);
        g = img(prototype, img_flt, req_threads, frame, G);
        b = img(prototype, img_flt, req_threads, frame, B);
    }

    img_rgb(const img_rgb<T> &other) = default;

    void write(const std::string &out_path)
    {
        cimg_library::CImg<img_t> img_flt(r.cols(), r.rows(), depth, 3);

        r.write(img_flt, depth - 1, R);
        g.write(img_flt, depth - 1, G);
        b.write(img_flt, depth - 1, B);

        clib::write_img(img_flt, out_path);
    }

    void write(cimg_library::CImg<img_t> &img_flt, idx_t frame)
    {
        r.write(img_flt, frame, R);
        g.write(img_flt, frame, G);
        b.write(img_flt, frame, B);
    }

    // rows - height of image
    idx_t rows() const
    {
        return r.rows();
    }

    // cols - width of image
    idx_t cols() const
    {
        return r.cols();
    }

    const img<T> &get_r() const
    {
        return r;
    }

    const img<T> &get_g() const
    {
        return g;
    }

    const img<T> &get_b() const
    {
        return b;
    }

    img_rgb operator+(const T &rhs) const
    {
        img_rgb<T> res;

        res.r = r + rhs;
        res.g = g + rhs;
        res.b = b + rhs;

        return res;
    }

    img_rgb operator*(const T &rhs) const
    {
        img_rgb<T> res;

        res.r = r * rhs;
        res.g = g * rhs;
        res.b = b * rhs;

        return res;
    }

    img_rgb operator-(const T &rhs) const
    {
        img_rgb<T> res;

        res.r = r - rhs;
        res.g = g - rhs;
        res.b = b - rhs;

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

        res.r = r + rhs.r;
        res.g = g + rhs.g;
        res.b = b + rhs.b;

        return res;
    }
    img_rgb operator*(const img_rgb &rhs) const
    {
        assert(cols() == rhs.cols());
        assert(rows() == rhs.rows());

        img_rgb res;

        res.r = r * rhs.r;
        res.g = g * rhs.g;
        res.b = b * rhs.b;

        return res;
    }
    img_rgb operator-(const img_rgb &rhs) const
    {
        assert(cols() == rhs.cols());
        assert(rows() == rhs.rows());

        img_rgb res;

        res.r = r - rhs.r;
        res.g = g - rhs.g;
        res.b = b - rhs.b;

        return res;
    }
};

template <typename T> img_rgb<T> operator+(const T &lhs, const img_rgb<T> &rhs)
{
    img_rgb<T> res;

    res.r = lhs + rhs.r;
    res.g = lhs + rhs.g;
    res.b = lhs + rhs.b;

    return res;
}

template <typename T> img_rgb<T> operator*(const T &lhs, const img_rgb<T> &rhs)
{
    img_rgb<T> res;

    res.r = lhs * rhs.r;
    res.g = lhs * rhs.g;
    res.b = lhs * rhs.b;

    return res;
}

template <typename T> img_rgb<T> operator-(const T &lhs, const img_rgb<T> &rhs)
{
    img_rgb<T> res;

    res.r = lhs - rhs.r;
    res.g = lhs - rhs.g;
    res.b = lhs - rhs.b;

    return res;
}

} // namespace clib
