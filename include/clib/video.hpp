#pragma once

#define cimg_use_png
#define cimg_use_jpeg

#include "CImg.h"
#include "image.hpp"
#include "video.hpp"

#include "VideoView.hpp"

namespace clib
{

using std::vector;
using pixel_t = VideoView::pixel_t;
using idx_t = VideoView::idx_t;
template <typename T> class video
{
    vector<img_rgb<T>> frames_;

  public:
    /*! @brief Инициализации изображения из Представления
     *
     * \param[in] prorotype Элемент, из которого берутся гиперпараметры
     * \param[in] view Представление видео
     */
    video(const T &prototype, const VideoView &view)
    {
        assert(view.rows() > 0);
        assert(view.cols() > 0);
        assert(view.clrs() == 3);
        assert(view.frames() > 0);

        idx_t nthreads = determine_threads(view.frames());

        frames_.reserve(view.frames());
        for (idx_t fr = 0; fr < view.frames(); ++fr)
            frames_.emplace_back(prototype, 1, 1);

        // разбиваем по потокам
        work(
            nthreads, view.frames(),
            [&](idx_t st_fr, idx_t en_fr, idx_t rows, idx_t cols) {
                for (idx_t fr = st_fr; fr < en_fr; ++fr)
                {
                    auto cur_img = img_rgb<T>(prototype, rows, cols, 1);
                    for (idx_t clr = 0; clr < 3; ++clr)
                        for (idx_t i = 0; i < rows; ++i)
                            for (idx_t j = 0; j < cols; ++j)
                            {
                                auto val = T::from_float(prototype, view.get(i, j, clr, fr));
                                cur_img(i, j, clr) = val;
                            }
                    frames_[fr] = std::move(cur_img);
                }
            },
            view.rows(), view.cols());

        // frames_.reserve(view.frames());
        // for (idx_t fr = 0; fr < view.frames(); ++fr)
        //     frames_.emplace_back(prototype, view.rows(), view.cols());

        // // разбиваем по потокам
        // work(nthreads, view.frames(), [&](idx_t st_fr, idx_t en_fr, idx_t rows, idx_t cols) {
        //     for (idx_t fr = st_fr; fr < en_fr; ++fr)
        //         for (idx_t clr = 0; clr < 3; ++clr)
        //             for (idx_t i = 0; i < rows; ++i)
        //                 for (idx_t j = 0; j < cols; ++j)
        //                 {
        //                     auto val = T::from_float(prototype, view.get(i, j, clr, fr));
        //                     frames_[fr](i, j, clr) = val;
        //                 }
        // }, view.rows(), view.cols());
    }
    video(const vector<img_rgb<T>> &frames)
    {
        frames_ = frames;
    }

    void write(VideoView &view)
    {
        assert(view.rows() == rows());
        assert(view.cols() == cols());
        assert(view.clrs() == 3);
        assert(view.frames() == frames());

        idx_t nthreads = determine_threads(view.frames());

        // разбиваем по потокам
        work(
            nthreads, view.frames(),
            [&](idx_t st_fr, idx_t en_fr, idx_t rows, idx_t cols) {
                for (idx_t fr = st_fr; fr < en_fr; ++fr)
                    for (idx_t clr = 0; clr < 3; ++clr)
                        for (idx_t i = 0; i < rows; ++i)
                            for (idx_t j = 0; j < cols; ++j)
                            {
                                auto val = frames_[fr](i, j, clr).to_float();
                                view.set(val, i, j, clr, fr);
                            }
            },
            view.rows(), view.cols());
    }

    // rows - height of image
    idx_t rows() const
    {
        return frames_[0].rows();
    }

    // cols - width of image
    idx_t cols() const
    {
        return frames_[0].cols();
    }

    idx_t frames() const
    {
        return static_cast<idx_t>(frames_.size());
    }

    const img_rgb<T> &operator()(idx_t i) const
    {
        assert(i < frames_.size());

        return frames_[i];
    }

    img_rgb<T> &operator()(idx_t i)
    {
        assert(i < frames_.size());

        return frames_[i];
    }

  private:
    // Выполняет func над this, разделяя работу на nthreads потоков
    // Пример использования в ctor
    template <typename Func, typename... Args> static void work(idx_t nthreads, idx_t frames, Func func, Args... args)
    {
        assert(nthreads > 0);
        assert(frames != 0);

        if (nthreads == 1)
        {
            std::invoke(func, 0, frames, args...);
            return;
        }

        vector<std::thread> threads(nthreads);
        idx_t bsize = std::max(frames / nthreads, 1u);

        /////////////////// Создаем потоки ///////////////////
        idx_t tidx = 0;
        idx_t last_fr = 0;
        for (; frames >= bsize * (tidx + 1) && tidx < nthreads; last_fr += bsize, tidx += 1)
            threads[tidx] = std::thread(func, last_fr, last_fr + bsize, args...);

        // Обрабатываем остаток работ
        auto remainder = frames - bsize * tidx;
        if (remainder > 0)
            std::invoke(func, last_fr, frames, args...);

        // Ждем потоки
        for (idx_t th = 0; th < tidx; ++th)
            threads[th].join();
    }

    // Определение оптимального количество потоков
    static idx_t determine_threads(idx_t req_threads)
    {
        assert(req_threads > 0);

        idx_t hard_conc = static_cast<idx_t>(std::thread::hardware_concurrency());
        return std::min(hard_conc != 0 ? hard_conc : 2, req_threads);
    }
    void measure_time(const std::string &msg)
    {
        static auto last_time = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - last_time).count();
        std::cout << msg + ": " << diff << "ms" << std::endl;

        last_time = std::chrono::steady_clock::now();
    }
};

#ifdef OLD_VERSION_VIDEO
cimg_library::CImg<pixel_t> read_video(const std::string &path);
void write_video(const cimg_library::CImg<pixel_t> &video, const std::string &path);
using pixel_t = ImgView::pixel_t;
using idx_t = ImgView::idx_t;
template <typename T> class video
{
    std::vector<img_rgb<T>> frames_;

  public:
    video(const T &prototype, const std::string &video_path)
    {
        cimg_library::CImg video_view = read_video(video_path);
        frames_.reserve(static_cast<idx_t>(video_view.depth()));

        auto view_r = clib::CImgView{};
        auto view_g = clib::CImgView{};
        auto view_b = clib::CImgView{};

        for (idx_t i = 0; i < static_cast<idx_t>(video_view.depth()); ++i)
        {
            view_r.load(video_view.get_shared_slice(i, ImgView::R));
            view_g.load(video_view.get_shared_slice(i, ImgView::G));
            view_b.load(video_view.get_shared_slice(i, ImgView::B));

            // std::cout << i << std::endl;
            auto r = img<T>(prototype, view_r);
            auto g = img<T>(prototype, view_g);
            auto b = img<T>(prototype, view_b);

            frames_.push_back(img_rgb(r, g, b));
        }
    }

    video(const std::vector<img_rgb<T>> &frames) : frames_(frames)
    {
    }

    void write(const std::string &video_path)
    {
        cimg_library::CImg<pixel_t> video_view(cols(), rows(), static_cast<idx_t>(frames_.size()), 3);

        auto view = clib::CImgView{};
        view.init(rows(), cols(), 3);
        for (idx_t i = 0; i < depth(); ++i)
        {
            frames_[i].write(view);
            video_view.get_shared_slice(i, 0) = view.unload().get_shared_slice(0, 0);
            video_view.get_shared_slice(i, 1) = view.unload().get_shared_slice(0, 1);
            video_view.get_shared_slice(i, 2) = view.unload().get_shared_slice(0, 2);
        }

        write_video(video_view, video_path);
    }

    // rows - height of image
    idx_t rows() const
    {
        return frames_[0].rows();
    }

    // cols - width of image
    idx_t cols() const
    {
        return frames_[0].cols();
    }

    idx_t depth() const
    {
        return static_cast<idx_t>(frames_.size());
    }

    const img_rgb<T> &operator()(idx_t i) const
    {
        assert(i < frames_.size());

        return frames_[i];
    }

    img_rgb<T> &operator()(idx_t i)
    {
        assert(i < frames_.size());

        return frames_[i];
    }
};
#endif

} // namespace clib