#pragma once

#define cimg_use_png
#define cimg_use_jpeg

#include "CImg.h"
#include "image.hpp"
#include "video.hpp"

namespace clib
{

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

            //std::cout << i << std::endl;
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

} // namespace clib