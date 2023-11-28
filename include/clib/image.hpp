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
template <typename T> 
class img final
{
    vector<vector<T>> vv;

    size_t rows_ = 0;
    size_t cols_ = 0;

    T inv_rows_{};
    T inv_cols_{};

  public:
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

        vv.reserve(rows_);
        vv.resize(rows_);

        size_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        // разбиваем по потокам
        work(nthreads, &img::init_from_img_impl, std::ref(prototype), std::ref(img_flt));

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

        vv.reserve(rows_);
        vv.resize(rows_);

        size_t nthreads = req_threads;
        if (req_threads == 0)
            nthreads = determine_threads(MIN_THREAD_WORK);

        // разбиваем по потокам
        work(nthreads, &img::init_from_vv_impl, std::ref(base));

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
                img_flt(j, i) = vv[i][j].to_float();

        clib::write_img(img_flt, out_path);
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

    const vector<vector<T>> get_vv() const
    {
        return vv;
    }

    const size_t rows() const
    {
        return rows_;
    }

    const size_t cols() const
    {
        return cols_;
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

    void init_from_img_impl(size_t st_row, size_t en_row, const T &prototype, const cimg_library::CImg<IMG_T> &img_flt)
    {
        assert(cols_ != 0);
        assert(en_row >= st_row);

        for (auto i = st_row; i < en_row; ++i)
        {
            vv[i].reserve(cols_);
            for (size_t j = 0; j < cols_; ++j)
                vv[i].push_back(T::from_float(prototype, img_flt(j, i)));
        }
    }

    void init_from_vv_impl(size_t st_row, size_t en_row, const vector<vector<T>> &base)
    {
        assert(cols_ != 0);
        assert(en_row >= st_row);

        for (auto i = st_row; i < en_row; ++i)
        {
            vv[i].reserve(cols_);
            for (size_t j = 0; j < cols_; ++j)
                vv[i].push_back(base[i][j]);
        }
    }

    void mean_impl(size_t st_row, size_t en_row, vector<T> &res)
    {
        assert(en_row >= st_row);

        for (auto i = st_row; i < en_row; ++i)
        {
            T sum_even = T::from_float(vv[i][0], 0);
            T sum_odd = T::from_float(vv[i][0], 0);
            for (size_t j = 0; j < cols_; j += 2)
            {
                T::sum(vv[i][j], sum_even, sum_even);
                if (j + 1 < cols_)
                    T::sum(vv[i][j + 1], sum_odd, sum_odd);
            }
            T::sum(sum_even, sum_even, sum_odd);
            T::mult(inv_cols(), sum_even, sum_even);
            res[i] = sum_even;
        }
        std::flush(std::cout);
    }

    template <typename Func, typename... Args> void work(size_t nthreads, Func func, Args... args)
    {
        size_t rows = vv.size();
        assert(rows != 0);

        if (nthreads == 1)
        {
            std::invoke(func, this, 0, rows, args...);
            return;
        }

        vector<std::thread> threads(nthreads);
        size_t bsize = std::max(rows / nthreads, 1ul);

        /////////////////// Создаем потоки ///////////////////
        size_t tidx = 0;
        size_t last_row = 0;
        for (; rows >= bsize * (tidx + 1); last_row += bsize, tidx += 1)
        {
            std::cout << "tidx = " << tidx << std::endl;
            std::cout << "nthreads = " << nthreads << std::endl;
            std::flush(std::cout);

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
