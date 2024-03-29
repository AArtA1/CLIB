#pragma once

#include <iostream>
#include <stdexcept>

#include "Flexfloat.hpp"
#include "ImgView.hpp"
#include "common.hpp"
#include "logs.hpp"

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

    vector<vector<T>> vv_;

  public:
    img &operator=(const img &in)
    {
        rows_ = in.rows_;
        cols_ = in.cols_;
        vv_ = in.vv_;

        return *this;
    }

    img() = default;

    /*! @brief Инициализации изображения из другого изображения
     */
    img(const img<T> &base, idx_t req_threads = 0) : vv_()
    {
        // assert(base.rows() > 0);
        // assert(base.cols() > 0);

        auto get_val = [&base](idx_t i, idx_t j) { return base(i, j); };
        _ctor_implt(base.rows(), base.cols(), get_val, req_threads);
    }

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
            return T::from_arithmetic_t(prototype, view.get(i, j, clr));
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

        auto rows = base.size();
        auto cols = base[0].size();
        auto get_val = [&base](idx_t i, idx_t j) { return base[i][j]; };

        _ctor_implt(rows, cols, get_val, req_threads);
    }

    img(vector<vector<T>> &&base) : vv_()
    {
        assert(base.size() > 0);
        assert(base[0].size() > 0);

        rows_ = base.size();
        cols_ = base[0].size();

        vv_ = std::move(base);
    }

    /*! @brief Инициализации изображения массивом
     *
     * TODO
     */
    img(const img &base, const clib::Flexfloat::hyper_params &params, idx_t req_threads = 0) : vv_()
    {
        assert(base.vv_.size() > 0);

        auto rows = base.vv_.size();
        auto cols = base.vv_[0].size();
        auto get_val = [&base, &params](idx_t i, idx_t j) { return clib::Flexfloat::pack(base.vv_[i][j], params); };

        _ctor_implt(rows, cols, get_val, req_threads);
    }

    const vector<vector<T>> vv() const
    {
        return vv_;
    }

    vector<vector<T>> vv()
    {
        return vv_;
    }

    idx_t rows() const
    {
        return vv_.size();
    }
    idx_t cols() const
    {
        return vv_.empty() ? 0 : vv_[0].size();
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
        assert(cols_ != 0);
        assert(rows_ != 0);

        const idx_t modulus = 3;

#ifndef SUM_FIX
        // brute calculating of sum
        if (cols_ < modulus || rows_ < modulus)
        {
            T sum = T::from_arithmetic_t(vv_[0][0], 0.0f);
            for (idx_t i = 0; i < rows_; ++i)
            {
                for (idx_t j = 0; j < cols_; ++j)
                {
                    T::sum(sum, vv_[i][j], sum);
                }
            }
            return sum;
        }
#endif

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

        // The code below only works with correct implemented INVERSION function

        T volume = T::from_arithmetic_t(summ, rows_ * cols_);
        T inv_volume = T::from_arithmetic_t(summ, 0lu);
        T::inv(volume, inv_volume);
        T::mult(summ, inv_volume, summ);

        // comment this if INVERSION works correctly
        // summ = T::from_arithmetic_t(summ,  / (rows_ * cols_));

#ifdef BOOST_LOGS
        CLOG(debug) << "Mean:" << summ << " Float:" << summ.to_float();
        CLOG(debug) << "Volume:" << (rows_ * cols_);
        CLOG(debug) << "Volume inv_value: " << summ;
#endif

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
            if (vv_[i][j].to_int() < minn)
                res.vv_[i][j] = T::from_arithmetic_t(vv_[i][j], minn);
            else if (vv_[i][j].to_int() > maxx)
                res.vv_[i][j] = T::from_arithmetic_t(vv_[i][j], maxx);
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
    static void add(const img<T> &lhs, const img<T> &rhs, img<T> &res)
    {
        assert((rhs.rows_ == lhs.rows_) == res.rows_);
        assert((rhs.cols_ == lhs.cols_) == res.cols_);

        for_each(res.rows(), res.cols(),
                 [&](idx_t i, idx_t j) { T::sum(lhs.vv_[i][j], rhs.vv_[i][j], res.vv_[i][j]); });
    }
    static void add(const img<T> &lhs, const T &rhs, img<T> &res)
    {
        assert(lhs.rows_ == res.rows_);
        assert(lhs.cols_ == res.cols_);

        for_each(res.rows(), res.cols(), [&](idx_t i, idx_t j) { T::sum(lhs.vv_[i][j], rhs, res.vv_[i][j]); });
    }
    static void add(const T &lhs, const img<T> &rhs, img<T> &res)
    {
        assert(rhs.rows_ == res.rows_);
        assert(rhs.cols_ == res.cols_);

        for_each(res.rows(), res.cols(), [&](idx_t i, idx_t j) { T::sum(lhs, rhs.vv_[i][j], res.vv_[i][j]); });
    }
    img operator*(const T &rhs) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) { T::mult(res.vv_[i][j], rhs, res.vv_[i][j]); });

        return res;
    }
    static void mult(const img<T> &lhs, const img<T> &rhs, img<T> &res)
    {
        assert((rhs.rows_ == lhs.rows_) == res.rows_);
        assert((rhs.cols_ == lhs.cols_) == res.cols_);

        for_each(res.rows(), res.cols(),
                 [&](idx_t i, idx_t j) { T::mult(lhs.vv_[i][j], rhs.vv_[i][j], res.vv_[i][j]); });
    }
    static void mult(const img<T> &lhs, const T &rhs, img<T> &res)
    {
        assert(lhs.rows_ == res.rows_);
        assert(lhs.cols_ == res.cols_);

        for_each(res.rows(), res.cols(), [&](idx_t i, idx_t j) { T::mult(lhs.vv_[i][j], rhs, res.vv_[i][j]); });
    }
    static void mult(const T &lhs, const img<T> &rhs, img<T> &res)
    {
        assert(rhs.rows_ == res.rows_);
        assert(rhs.cols_ == res.cols_);

        for_each(res.rows(), res.cols(), [&](idx_t i, idx_t j) { T::mult(lhs, rhs.vv_[i][j], res.vv_[i][j]); });
    }

    img operator-(const T &rhs) const
    {
        img res(*this);
        for_each(rows_, cols_, [&](idx_t i, idx_t j) { T::sub(res.vv_[i][j], rhs, res.vv_[i][j]); });

        return res;
    }
    static void sub(const img<T> &lhs, const img<T> &rhs, img<T> &res)
    {
        assert((rhs.rows_ == lhs.rows_) == res.rows_);
        assert((rhs.cols_ == lhs.cols_) == res.cols_);

        for_each(res.rows(), res.cols(),
                 [&](idx_t i, idx_t j) { T::sub(lhs.vv_[i][j], rhs.vv_[i][j], res.vv_[i][j]); });
    }
    static void sub(const img<T> &lhs, const T &rhs, img<T> &res)
    {
        assert(lhs.rows_ == res.rows_);
        assert(lhs.cols_ == res.cols_);

        for_each(res.rows(), res.cols(), [&](idx_t i, idx_t j) { T::sub(lhs.vv_[i][j], rhs, res.vv_[i][j]); });
    }
    static void sub(const T &lhs, const img<T> &rhs, img<T> &res)
    {
        assert(rhs.rows_ == res.rows_);
        assert(rhs.cols_ == res.cols_);

        for_each(res.rows(), res.cols(), [&](idx_t i, idx_t j) { T::sub(lhs, rhs.vv_[i][j], res.vv_[i][j]); });
    }

    img operator/(const T &rhs) const
    {
        img res(*this);

        // The code below only works with correct implemented INVERSION function
        // T inv_rhs(rhs);
        // T::inv(rhs, inv_rhs);

        const T inv_rhs = T::from_arithmetic_t(rhs, 1.0f / rhs.to_float()); // comment this if INVERSION works correctly

        for_each(rows_, cols_, [&](idx_t i, idx_t j) { T::mult(res.vv_[i][j], inv_rhs, res.vv_[i][j]); });
        return res;
    }
    static void inv(const img<T> &x, img<T> &res)
    {
        assert(x.rows_ == res.rows_);
        assert(x.cols_ == res.cols_);

        for_each(res.rows(), res.cols(), [&](idx_t i, idx_t j) { T::inv(x.vv_[i][j], res.vv_[i][j]); });
    }
    static void inv(const T &x, T &res)
    {
        T::inv(x, res);
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
        // The code below only works with correct implemented INVERSION function

        // T inverted(res.vv_[0][0]);

        for_each(rows_, cols_, [&](idx_t i, idx_t j) {
            // T::inv(rhs.vv_[i][j], inverted);

            const T inverted = T::from_arithmetic_t(
                rhs.vv_[i][j], 1.0f / rhs.vv_[i][j].to_float()); // comment this if INVERSION works correctly

            T::mult(res.vv_[i][j], inverted, res.vv_[i][j]);
        });

        return res;
    }

    static img<T> abs(const img<T> &image)
    {
        assert(image.rows() != 0);
        assert(image.cols() != 0);
        img<T> res(image(0, 0), image.rows(), image.cols());

        img<T>::for_each(image.rows(), image.cols(), [&](idx_t i, idx_t j) { T::abs(image(i, j), res(i, j)); });

        return res;
    }

    inline T &operator()(idx_t i, idx_t j)
    {
        assert(i < vv_.size());
        assert(j < vv_[0].size());
        return vv_[i][j];
    }
    inline const T &operator()(idx_t i, idx_t j) const
    {
        assert(i < vv_.size());
        assert(j < vv_[0].size());
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
                view.set(vv_[i][j].to_int(), i, j, clr);
    }

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

    #ifdef DEPRECATED_METHODS

    static std::pair<idx_t, idx_t> transform_coordinates(const vector<vector<T>> &vv_, std::pair<int, int> coordinates,
                                                         std::pair<int, int> center)
    {
        if (coordinates.first < 0)
        {
            coordinates.first = 2 * center.first - coordinates.first;
        }
        assert(vv_.size() < std::numeric_limits<int>::max());
        if (coordinates.first >= narrow_cast<int>(vv_.size()))
        {
            coordinates.first = 2 * center.first - coordinates.first;
        }
        if (coordinates.second < 0)
        {
            coordinates.second = 2 * center.second - coordinates.second;
        }
        assert(vv_[0].size() < std::numeric_limits<int>::max());
        if (coordinates.second >= narrow_cast<int>(vv_[0].size()))
        {
            coordinates.second = 2 * center.second - coordinates.second;
        }
        return coordinates;
    }

    static img<T> get_window(const img<T> &image, std::pair<idx_t, idx_t> center, std::pair<idx_t, idx_t> shape)
    {
        assert(center.first < image.rows());
        assert(center.second < image.cols());

        assert(shape.first < image.rows());
        assert(shape.first % 2 == 1);
        assert(shape.second < image.cols());
        assert(shape.second % 2 == 1);

        int top = narrow_cast<int>(center.first - shape.first / 2);
        int left = narrow_cast<int>(center.second - shape.second / 2);

        vector<vector<T>> res_window(shape.first, vector<T>(shape.second));

        // Copy elements from the original matrix to the new one
        for (size_t i = 0; i < shape.first; ++i)
        {
            for (size_t j = 0; j < shape.second; ++j)
            {
                auto res_coord = transform_coordinates(
                    image.vv_, {top + narrow_cast<int>(i), left + narrow_cast<int>(j)}, {center.first, center.second});
                res_window[i][j] = image.vv_[res_coord.first][res_coord.second];
            }
        }

        std::move(res_window);
        return img<T>(static_cast<std::vector<std::vector<T>> &&>(res_window));
    }

    static std::vector<std::vector<img<T>>> window(const img<T>& image, std::pair<idx_t, idx_t> shape){
        std::vector<std::vector<img<T>>> res;
        return res;
    }

    #endif

    static std::vector<std::vector<img<T>>> get_window(const img<T> &image, std::pair<idx_t, idx_t> shape)
    {
        assert(shape.first < image.rows());
        assert(shape.first % 2 == 1);
        assert(shape.second < image.cols());
        assert(shape.second % 2 == 1);

        idx_t h = image.rows();
        idx_t w = image.cols();

        idx_t dh = shape.first - 1, dw = shape.second - 1;

        idx_t di = dh / 2, dj = dw / 2;

        auto y = std::vector<std::vector<T>>(h + dh, std::vector<T>(w + dw));

        auto mirror_index = [](idx_t x_first, idx_t x_second, idx_t x_max) -> idx_t {
            idx_t temp;
            if (x_first > x_second + x_max)
            {
                temp = x_first - x_second - x_max;
            }
            else
            {
                temp = x_second + x_max - x_first;
            }
            return temp > x_max ? temp - x_max : x_max - temp;
        };

        for (idx_t i = 0; i < h + 2 * di; ++i)
        {
            for (idx_t j = 0; j < w + 2 * dj; ++j)
            {
                y[i][j] = image(mirror_index(i, di, h - 1), mirror_index(j, dj, w - 1));
            }
        }

        std::vector<std::vector<img<T>>> res;

        res.reserve(shape.first);

        img<T> y_img(std::move(y));

        // std::cout << std::endl;

        // for(auto vec : y_img.vv()){
        //     for(auto it : vec){
        //         std::cout << it.to_float() << "  ";
        //     }
        //     std::cout << std::endl;
        // }

        for (idx_t i = 0; i < shape.first; ++i)
        {
            std::vector<img<T>> row;
            row.reserve(shape.second);
            for (idx_t j = 0; j < shape.second; ++j)
            {
                row.push_back(get_subimg(y_img, i, i + h, j, j + w));
            }
            res.push_back(row);
        }

        return res;
    }

#ifdef DEPRECATED_METHODS

    static img<T> convolution(const img<T> &image, std::pair<idx_t, idx_t> shape, std::function<T(const img<T> &)> func)
    {
        assert(image.rows() != 0 && image.cols() != 0);
        img<T> res(image(0, 0), image.rows(), image.cols());
        img<T>::for_each(image.rows(), image.cols(), [&](idx_t i, idx_t j) {
            res.vv_[i][j] = func(get_window(image, std::make_pair(i, j), shape));
        });
        return res;
    }

#endif

    static img<T> convolution(const std::vector<std::vector<img<T>>> &left,
                              const std::vector<std::vector<img<T>>> &right)
    {
        const T ZERO = T::from_arithmetic_t(left[0][0](0, 0), 0);

        img<T> res(ZERO, right[0][0].rows(), right[0][0].cols());

        // img<T> res(ZERO, right[0][0].rows(),right[0][0].cols());

        // think about multithreading
        for (idx_t i = 0; i < left.size(); ++i)
        {
            for (idx_t j = 0; j < left[0].size(); ++j)
            {
                assert((left[i][j].cols() == 1 && left[i][j].rows() == 1) ||
                       (left[i][j].rows() == right[i][j].rows() && left[i][j].cols() == right[i][j].cols()));
                if (left[i][j].rows() == 1 && left[i][j].cols() == 1)
                {
                    res = res + left[i][j](0, 0) * right[i][j];
                }
                else
                {
                    res = res + left[i][j] * right[i][j];
                }
                // debug_vecvec(res.vv());
            }
        }

        return res;
    }

    // DEPRECATED
    // static img<T> transform(const img<T>& left, const img<T>& right, const std::func<(const &T,const T&,T&)){
    //     assert(left.rows() == right.rows());
    //     assert(left.cols() == right.cols());
    //     assert(left.rows() != 0 && left.cols() != 0);

    //     std::vector<std::vector<T>> result(left.rows(),std::vector<T>(left.cols()));

    //     img<T>::for_each(result.rows(), result.cols(), [&](idx_t i, idx_t j) {
    //         func(left(i,j),right(i,j),result(i,j));
    //     });

    //     return result;
    // }

    static img<T> condition(const std::vector<std::vector<bool>> &flags, const img<T> &true_val,
                            const img<T> &false_val)
    {
        assert(flags.size() == true_val.rows() && true_val.rows() == false_val.rows());
        assert(flags[0].size() == true_val.cols() && true_val.cols() == false_val.cols());

        img<T> result(true_val(0, 0), true_val.rows(), true_val.cols());

        img<T>::for_each(result.rows(), result.cols(),
                         [&](idx_t i, idx_t j) { result(i, j) = flags[i][j] ? true_val(i, j) : false_val(i, j); });

        return result;
    }

    static img<T> baer_matrix(const img<T> &r, const img<T> &g, const img<T> &b)
    {
        assert(r.rows() != 0 && r.rows() == g.rows() && g.rows() == b.rows());
        assert(r.cols() != 0 && r.cols() == g.cols() && g.cols() == b.cols());

        idx_t rows = r.rows(), cols = r.cols();

        img<T> baer(T::from_arithmetic_t(r(0, 0), 0), r.rows(), r.cols());

        for (idx_t i = 0; i < rows; ++i)
            for (idx_t j = 1 - (i % 2); j < cols; j += 2)
                baer(i, j) = g(i, j);

        for (idx_t i = 0; i < rows; i += 2)
            for (idx_t j = 0; j < cols; j += 2)
                baer(i, j) = r(i, j);

        for (idx_t i = 1; i < rows; i += 2)
            for (idx_t j = 1; j < cols; j += 2)
                baer(i, j) = b(i, j);

        return baer;
    }

    static std::vector<img<T>> baer_split(const img<T> &baer)
    {

        idx_t rows = baer.rows(), cols = baer.cols();

        img<T> r(T::from_arithmetic_t(baer(0, 0), 0), rows, cols);
        img<T> g(T::from_arithmetic_t(baer(0, 0), 0), rows, cols);
        img<T> b(T::from_arithmetic_t(baer(0, 0), 0), rows, cols);

        for (idx_t i = 0; i < rows; ++i)
            for (idx_t j = 1 - (i % 2); j < cols; j += 2)
                g(i, j) = baer(i, j);

        for (idx_t i = 0; i < rows; i += 2)
            for (idx_t j = 0; j < cols; j += 2)
                r(i, j) = baer(i, j);

        for (idx_t i = 1; i < rows; i += 2)
            for (idx_t j = 1; j < cols; j += 2)
                b(i, j) = baer(i, j);

        return {r, g, b};
    }

#define CREATE_T(param, value) T::from_arithmetic_t(param, value)

#define CREATE_IMG(param, value) img<T>({{CREATE_T(param, value)}})

    static std::vector<img<T>> demosaic(const img<T> &r, const img<T> &g, const img<T> &b)
    {

        const std::vector<std::vector<img<T>>> mx(
            {{CREATE_IMG(r(0, 0), 1), CREATE_IMG(r(0, 0), 0), CREATE_IMG(r(0, 0), -1)},
             {CREATE_IMG(r(0, 0), 2), CREATE_IMG(r(0, 0), 0), CREATE_IMG(r(0, 0), -2)},
             {CREATE_IMG(r(0, 0), 1), CREATE_IMG(r(0, 0), 0), CREATE_IMG(r(0, 0), -1)}});

        const std::vector<std::vector<img<T>>> my(
            {{CREATE_IMG(r(0, 0), 1), CREATE_IMG(r(0, 0), 2), CREATE_IMG(r(0, 0), 1)},
             {CREATE_IMG(r(0, 0), 0), CREATE_IMG(r(0, 0), 0), CREATE_IMG(r(0, 0), 0)},
             {CREATE_IMG(r(0, 0), -1), CREATE_IMG(r(0, 0), -2), CREATE_IMG(r(0, 0), 1)}});

        std::vector<std::vector<img<T>>> wg = get_window(g, {3, 3});

        img<T> dgx = abs(convolution(mx, wg));

        img<T> dgy = abs(convolution(my, wg));

        img<T> g_new = wg[1][1] + condition((dgx < dgy), (wg[1][0] + wg[1][2]) / (CREATE_T(r(0, 0), 2)),
                                            (wg[0][1] + wg[2][1]) / (CREATE_T(r(0, 0), 2)));

        // debug_vecvec(g_new.vv());

        const std::vector<std::vector<img<T>>> m(
            {{CREATE_IMG(r(0, 0), 1), CREATE_IMG(r(0, 0), 2), CREATE_IMG(r(0, 0), 1)},
             {CREATE_IMG(r(0, 0), 2), CREATE_IMG(r(0, 0), 4), CREATE_IMG(r(0, 0), 2)},
             {CREATE_IMG(r(0, 0), 1), CREATE_IMG(r(0, 0), 2), CREATE_IMG(r(0, 0), 1)}});

        auto g_lpf = convolution(m, wg) / T::from_arithmetic_t(r(0, 0), 8);
        auto r_lpf = convolution(m, get_window(r, {3, 3})) / T::from_arithmetic_t(r(0, 0), 4);
        auto b_lpf = convolution(m, get_window(b, {3, 3})) / T::from_arithmetic_t(r(0, 0), 4);

        // debug_vecvec(r_lpf.vv());

        // debug_vecvec(g_lpf.vv());

        // debug_vecvec(b_lpf.vv());

        // zero value
        const T ZERO = T::from_arithmetic_t(g_lpf(0, 0), 0);

        img<T> r_new = condition(g_lpf == img<T>(ZERO, g_lpf.rows(), g_lpf.cols()), r_lpf, g_new * r_lpf / g_lpf);

        img<T> b_new = condition(g_lpf == img<T>(ZERO, g_lpf.rows(), g_lpf.cols()), b_lpf, g_new * b_lpf / g_lpf);

        return {r_new, g_new, b_new};
    }

    static std::vector<std::vector<bool>> create_bool_mask(const img<T> &lhs, const img<T> &rhs,
                                                           std::function<bool(const T &, const T &)> compare)
    {
        assert(lhs.rows() == rhs.rows());
        assert(lhs.cols() == rhs.cols());

        std::vector<std::vector<bool>> flags(lhs.rows(), std::vector<bool>(lhs.cols()));

        img<T>::for_each(lhs.rows(), lhs.cols(),
                         [&](idx_t i, idx_t j) { flags[i][j] = compare(lhs(i, j), rhs(i, j)); });

        return flags;
    }

    template <typename U> friend std::vector<std::vector<bool>> operator>(const img<U> &lhs, const img<U> &rhs);

    template <typename U> friend std::vector<std::vector<bool>> operator>=(const img<U> &lhs, const img<U> &rhs);

    template <typename U> friend std::vector<std::vector<bool>> operator<(const img<U> &lhs, const img<U> &rhs);

    template <typename U> friend std::vector<std::vector<bool>> operator<=(const img<U> &lhs, const img<U> &rhs);

    template <typename U> friend std::vector<std::vector<bool>> operator==(const img<U> &lhs, const img<U> &rhs);

    template <typename U> friend std::vector<std::vector<bool>> operator!=(const img<U> &lhs, const img<U> &rhs);

#undef CREATE_IMG

#undef CREATE_T

  private:
    static img<T> get_subimg(const img<T> &initial, idx_t row_start, idx_t row_end, idx_t col_start, idx_t col_end)
    {
        std::vector<std::vector<T>> rect(row_end - row_start, std::vector<T>(col_end - col_start));
        for (idx_t r = row_start; r < row_end; r++)
            for (idx_t c = col_start; c < col_end; c++)
                rect[r - row_start][c - col_start] = initial(r, c);
        return img<T>(rect);
    }

    // Выполняет func над this, разделяя работу на nthreads потоков
    // Пример использования в mean
    template <typename Func, typename... Args> static void work(idx_t nthreads, idx_t rows, Func func, Args... args)
    {
        assert(nthreads > 0);
        assert(rows != 0);

        if (nthreads == 1)
        {
            func(0, rows, args...);
            return;
        }

        vector<std::thread> threads(nthreads);
        idx_t bsize = std::max(rows / nthreads, 1lu);

        /////////////////// Создаем потоки ///////////////////
        idx_t tidx = 0;
        idx_t last_row = 0;
        for (; rows >= bsize * (tidx + 1) && tidx < nthreads; last_row += bsize, tidx += 1)
            threads[tidx] = std::thread(func, last_row, last_row + bsize, args...);

        // Обрабатываем остаток работ
        auto remainder = rows - bsize * tidx;
        if (remainder > 0)
            func(last_row, rows, args...);

        // Ждем потоки
        for (idx_t th = 0; th < nthreads; ++th)
            threads[th].join();
    }

    // Определение оптимального количество потоков исходя из количества работы и параметров системы
    static idx_t determine_threads(idx_t rows, idx_t cols, idx_t min_thread_work)
    {
        assert(rows != 0);
        assert(cols != 0);

        idx_t rows_per_thread = std::max(min_thread_work / cols, 1lu);
        idx_t det_threads = std::max(rows / rows_per_thread, 1lu);

        idx_t hard_conc = std::thread::hardware_concurrency();
        return std::min(hard_conc != 0 ? hard_conc : 2, det_threads);
    }
    idx_t determine_threads(idx_t min_thread_work) const
    {
        return determine_threads(rows_, cols_, min_thread_work);
    }

    // Вспомогательная функция для определения оптимального количество потоков для параллелизма
    // static void determine_work_number(const T &prototype)
    // {
    //     struct test_res
    //     {
    //         long long t_init;
    //         long long t_calc;
    //         float mean;
    //     };
    //     auto calc = [&prototype](idx_t req_threads, idx_t rows, idx_t cols) {
    //         auto s_begin = std::chrono::steady_clock::now();

    //         auto one = T::from_arithmetic_t(prototype, 1);
    //         img im(one, rows, cols, req_threads);

    //         auto s_init = std::chrono::steady_clock::now();
    //         auto mean = im.mean();
    //         auto s_end = std::chrono::steady_clock::now();

    //         auto t_init = std::chrono::duration_cast<std::chrono::microseconds>(s_init - s_begin).count();
    //         auto t_calc = std::chrono::duration_cast<std::chrono::microseconds>(s_end - s_init).count();

    //         test_res tr = {t_init, t_calc, mean.to_float()};
    //         return tr;
    //     };

    //     const idx_t tests_num = 33;
    //     const idx_t means_num = 40;
    //     const idx_t rows = 500;
    //     const idx_t cols = 500;

    //     // Прогреваем кэши
    //     for (idx_t i = 0; i < tests_num; i += 4)
    //         calc(i, rows, cols);

    //     std::vector<test_res> results;
    //     for (idx_t i = 0; i < tests_num; ++i)
    //     {
    //         // усреднение
    //         test_res cur_res = {0, 0, 0};
    //         for (idx_t j = 0; j < means_num; ++j)
    //         {
    //             test_res temp_res = calc(i, rows, cols);
    //             cur_res.t_init += temp_res.t_init;
    //             cur_res.t_calc += temp_res.t_calc;
    //             cur_res.mean = temp_res.mean;
    //         }
    //         results.push_back({cur_res.t_init / means_num, cur_res.t_calc / means_num, cur_res.mean});
    //     }
    // }
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

template <typename T> std::vector<std::vector<bool>> operator>(const img<T> &lhs, const img<T> &rhs)
{
    return img<T>::create_bool_mask(lhs, rhs, [](const T &left, const T &right) { return left > right; });
}

template <typename T> std::vector<std::vector<bool>> operator>=(const img<T> &lhs, const img<T> &rhs)
{
    return img<T>::create_bool_mask(lhs, rhs, [](const T &left, const T &right) { return left >= right; });
}

template <typename T> std::vector<std::vector<bool>> operator<(const img<T> &lhs, const img<T> &rhs)
{
    return img<T>::create_bool_mask(lhs, rhs, [](const T &left, const T &right) { return left < right; });
}

template <typename T> std::vector<std::vector<bool>> operator<=(const img<T> &lhs, const img<T> &rhs)
{
    return img<T>::create_bool_mask(lhs, rhs, [](const T &left, const T &right) { return left <= right; });
}

template <typename T> std::vector<std::vector<bool>> operator==(const img<T> &lhs, const img<T> &rhs)
{
    return img<T>::create_bool_mask(lhs, rhs, [](const T &left, const T &right) { return left == right; });
}

template <typename T> std::vector<std::vector<bool>> operator!=(const img<T> &lhs, const img<T> &rhs)
{
    return img<T>::create_bool_mask(lhs, rhs, [](const T &left, const T &right) { return left != right; });
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
    img_rgb() = default;

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
        : r_(prototype, view, ImgView::R, req_threads), g_(prototype, view, ImgView::G, req_threads),
          b_(prototype, view, ImgView::B, req_threads)
    {
        assert(view.clrs() == 3);
    }

    /*! @brief Инициализации изображения одинаковыми значениями
     *
     * \param[in] prorotype Элемент, из которого берутся гиперпараметры
     * \param[in] rows Количество строк
     * \param[in] cols Количество столбцов
     */
    img_rgb(const T &prototype, idx_t rows, idx_t cols, idx_t req_threads = 0)
        : r_(prototype, rows, cols, req_threads), g_(prototype, rows, cols, req_threads),
          b_(prototype, rows, cols, req_threads)
    {
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
    img<T> &r()
    {
        return r_;
    }
    img<T> &g()
    {
        return g_;
    }
    img<T> &b()
    {
        return b_;
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

    T &operator()(idx_t i, idx_t j, idx_t clr)
    {
        assert(clr < 3);

        if (clr == ImgView::R)
        {
            assert(i < r_.rows());
            assert(j < r_.cols());
            return r_(i, j);
        }
        if (clr == ImgView::G)
        {
            assert(i < g_.rows());
            assert(j < g_.cols());
            return g_(i, j);
        }
        if (clr == ImgView::B)
        {
            assert(i < b_.rows());
            assert(j < b_.cols());
            return b_(i, j);
        }

        throw std::runtime_error{"Unreachable path"};
    }
    const img<T> &operator[](idx_t clr) const
    {
        assert(clr < 3);

        if (clr == ImgView::R)
            return r_;
        if (clr == ImgView::G)
            return g_;
        if (clr == ImgView::B)
            return b_;

        throw std::runtime_error{"Unreachable path"};
    }

    img<T> &operator[](idx_t clr)
    {
        assert(clr < 3);

        if (clr == ImgView::R)
            return r_;
        if (clr == ImgView::G)
            return g_;
        if (clr == ImgView::B)
            return b_;

        throw std::runtime_error{"Unreachable path"};
    }
    const T &operator()(idx_t i, idx_t j, idx_t clr) const
    {
        assert(clr < 3);

        if (clr == ImgView::R)
        {
            assert(i < r_.rows());
            assert(j < r_.cols());
            return r_(i, j);
        }
        if (clr == ImgView::G)
        {
            assert(i < g_.rows());
            assert(j < g_.cols());
            return g_(i, j);
        }
        if (clr == ImgView::B)
        {
            assert(i < b_.rows());
            assert(j < b_.cols());
            return b_(i, j);
        }

        throw std::runtime_error{"Unreachable path"};
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
