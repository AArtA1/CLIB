#pragma once

#define cimg_use_png
#define cimg_use_jpeg


#include "CImg.h"
#include "X11/Xlib.h"

extern "C" {
    #include <libavformat/avformat.h>
}

#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

namespace clib
{

const std::vector<std::string> video_extensions = {"mp4","mkv"};

/*!
 * \brief Обёртка над классом для работы с видео
 *
 * \details Абстрагирует любую библиотеку в набор методов
 */
class VideoView
{
protected:
    size_t fps_;

public:
    using pixel_t = int;
    using idx_t = unsigned;

    // colors number
    enum spectrum
    {
        R = 0,
        G = 1,
        B = 2
    };

    virtual void init(idx_t rows, idx_t cols, idx_t colors, idx_t frames, size_t fps) = 0;

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

class CVideoView : public VideoView
{
    cimg_library::CImg<pixel_t> video_;
    bool video_created_ = false;

public:

    void init(idx_t rows, idx_t cols, idx_t colors, idx_t frames, size_t fps) override;

    idx_t rows() const override;
    
    idx_t cols() const override;
    
    idx_t clrs() const override;

    idx_t frames() const override;

    // CImg stores data as [width,height]. Therefore, the data in cimg are transposed
    pixel_t get(idx_t i, idx_t j, idx_t clr, idx_t frame) const override;

    // CImg stores data as [width,height]. Therefore, the data in cimg are transposed
    void set(pixel_t val, idx_t i, idx_t j, idx_t clr, idx_t frame) override;

    void read_video(const std::string &path);

    void write_video(const std::string &path);

private:
    void check_created() const;
};


size_t get_fps(const std::string& path);
bool check_ext(const std::string &s, const std::vector<std::string> &exts);

} // namespace clib
