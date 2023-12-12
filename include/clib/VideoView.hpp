#pragma once

#define cimg_use_png
#define cimg_use_jpeg

#include "CImg.h"
#include "X11/Xlib.h"

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

namespace clib
{

/*!
 * \brief Обёртка над классом для работы с видео
 *
 * \details Абстрагирует любую библиотеку в набор методов
 */
struct VideoView
{
    using pixel_t = float;
    using idx_t = unsigned;

    // colors number
    enum spectrum
    {
        R = 0,
        G = 1,
        B = 2
    };

    virtual idx_t rows() const = 0;   // height
    virtual idx_t cols() const = 0;   // width
    virtual idx_t clrs() const = 0;   // количество цветов
    virtual idx_t frames() const = 0; // количество кадров

    // Работа с пикселями
    virtual pixel_t get(idx_t i, idx_t j, idx_t clr, idx_t frame) const = 0;
    virtual void set(pixel_t val, idx_t i, idx_t j, idx_t clr, idx_t frame) = 0;

    virtual void read_video(const std::string &path) = 0;
    virtual void write_video(const std::string &path) = 0;

    virtual ~VideoView()
    {
    }
};

struct CVideoView : VideoView
{

  private:
    cimg_library::CImg<pixel_t> video_;
    bool video_created_ = false;

  public:
    idx_t rows() const override
    {
        check_created();
        return static_cast<idx_t>(video_.height());
    }
    idx_t cols() const override
    {
        check_created();
        return static_cast<idx_t>(video_.width());
    }
    idx_t clrs() const override
    {
        check_created();
        return static_cast<idx_t>(video_.spectrum());
    }
    idx_t frames() const override
    {
        check_created();
        return static_cast<idx_t>(video_.depth());
    }

    // CImg stores data as [width,height]. Therefore, the data in cimg are transposed
    pixel_t get(idx_t i, idx_t j, idx_t clr, idx_t frame) const override
    {
        assert(i < rows());
        assert(j < cols());
        assert(clr < clrs());
        assert(frame < frames());

        check_created();
        return video_(j, i, frame, clr);
    }

    // CImg stores data as [width,height]. Therefore, the data in cimg are transposed
    void set(pixel_t val, idx_t i, idx_t j, idx_t clr, idx_t frame) override
    {
        assert(i < rows());
        assert(j < cols());
        assert(clr < clrs());
        assert(frame < frames());

        check_created();
        video_(j, i, frame, clr) = val;
    }

    void read_video(const std::string &path)
    {
        if (check_ext(path, {"mp4", "mkv"}))
            video_ = cimg_library::CImg<pixel_t>::get_load_video(path.c_str());
        else
            throw std::invalid_argument("Unknown format: " + path);

        video_created_ = true;
    }

    void write_video(const std::string &path)
    {
        if (check_ext(path, {"mp4", "mkv"}))
            video_.save_video(path.c_str());
        else
            throw std::invalid_argument("Unknown format: " + path);
    }

  private:
    static bool check_ext(const std::string &s, const std::vector<std::string> &exts)
    {
        for (auto ext : exts)
            if (s.substr(s.find_last_of(".") + 1) == ext)
                return true;
        return false;
    }

    void check_created() const
    {
        if (!video_created_)
            throw std::runtime_error{"video was not created"};
    }
};

} // namespace clib
