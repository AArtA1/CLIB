#pragma once

#include "CImg.h"
#include "video.hpp"
#include "image.hpp"

namespace clib
{

// ----------------------- SUPPORTS ONLY MP4, MKV extensions --------------------

template <typename T> cimg_library::CImg<T> read_video(const std::string &path)
{
    cimg_library::CImg<T> video;
    if (check_ext(path, {"mp4", "mkv"}))
    {

        video = cimg_library::CImg<T>::get_load_video(path.c_str());
    }
    else
    {
#ifndef NDEBUG
        CLOG(error) << "Unknown format: " << path.substr(path.find_last_of(".") + 1) << std::endl;
#endif
        throw std::invalid_argument("Unknown format. Please check again");
    }
    return video;
}

template <typename T> void write_video(const cimg_library::CImg<T> &video, const std::string &path)
{
    if (check_ext(path, {
                            "mp4",
                            "mkv",
                        }))
    {
        video.save_video(path.c_str());
    }
    else
    {
#ifndef NDEBUG
        CLOG(error) << "Unknown format: " << path.substr(path.find_last_of(".") + 1) << std::endl;
#endif
        throw std::invalid_argument("Unknown format. Please check again");
    }
}

// -----------------------------------------------------------------------

template <typename T> class video
{
    std::vector<img_rgb<T>> frames;

  public:
    video() = default;

    video(const T &prototype, const std::string &video_path, idx_t req_threads = 0)
    {
        cimg_library::CImg img_flt = read_video<img_t>(video_path);
        frames.reserve(img_flt.depth());
        for (idx_t i = 0; i < img_flt.depth(); ++i)
        {
            frames.push_back(img_rgb(prototype, img_flt, i, req_threads));
        }
    }

    video(std::vector<img_rgb<T>> frames_) : frames(frames_)
    {
    }

    void write(const std::string &video_path)
    {
        cimg_library::CImg<img_t> img_flt(cols(), rows(), frames.size(), 3);

        for (idx_t i = 0; i < depth(); ++i)
        {
            frames[i].write(img_flt, i);
        }

        write_video(img_flt, video_path);
    }

    // rows - height of image
    idx_t rows() const
    {
        return frames[0].rows();
    }

    // cols - width of image
    idx_t cols() const
    {
        return frames[0].cols();
    }

    idx_t depth() const
    {
        return static_cast<idx_t>(frames.size());
    }

    const img_rgb<T> &operator()(idx_t i) const
    {
        assert(i < frames.size());

        return frames[i];
    }

    img_rgb<T> &operator()(idx_t i)
    {
        assert(i < frames.size());

        return frames[i];
    }
};

} // namespace clib