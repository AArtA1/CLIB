#pragma once

#define cimg_use_png
#define cimg_use_jpeg

#include "CImg.h"
#include "X11/Xlib.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <cassert>

namespace clib
{

using img_t = float;
using idx_t = unsigned;
struct ImgView
{
    virtual void init(idx_t rows, idx_t cols) = 0;
    virtual void init(idx_t rows, idx_t cols, idx_t frames, idx_t clrs) = 0;

    virtual idx_t rows() const = 0;
    virtual idx_t cols() const = 0;
    virtual idx_t frames() const = 0;
    virtual idx_t clrs() const = 0;

    virtual img_t get(idx_t i, idx_t j, idx_t frame, idx_t clr) const = 0;
    virtual void set(img_t val, idx_t i, idx_t j, idx_t frame, idx_t clr) = 0;

    virtual void read_img(const std::string &path) = 0;
    virtual void write_img(const std::string &path) = 0;

    virtual ~ImgView()
    {
    }
};

struct CImg : ImgView
{
  private:
    cimg_library::CImg<img_t> image_;
    bool img_was_readed_ = false;

  public:
    void init(idx_t rows, idx_t cols) override
    {
        image_ = cimg_library::CImg<img_t>(cols, rows);
    }
    void init(idx_t rows, idx_t cols, idx_t frames, idx_t clrs) override
    {
        image_ = cimg_library::CImg<img_t>(cols, rows, frames, clrs);
    }

    idx_t rows() const override
    {
        check_created();
        return static_cast<idx_t>(image_.height());
    }
    idx_t cols() const override
    {
        check_created();
        return static_cast<idx_t>(image_.width());
    }
    idx_t frames() const override
    {
        check_created();
        return static_cast<idx_t>(image_.depth());
    }
    idx_t clrs() const override
    {
        check_created();
        return static_cast<idx_t>(image_.spectrum());
    }


    img_t get(idx_t i, idx_t j, idx_t frame, idx_t clr) const override
    {
        check_created();
        return image_(j, i, frame, clr);
    }


    void set(img_t val, idx_t i, idx_t j, idx_t frame, idx_t clr) override
    {
        check_created();
        assert(i < static_cast<idx_t>(image_.height()));
        assert(j < static_cast<idx_t>(image_.width()));
        
        image_(j, i, frame, clr) = val;
    }

    void read_img(const std::string &path) override
    {
        if (check_ext(path, {"jpeg", "jpg"}))
            image_ = cimg_library::CImg<img_t>::get_load_jpeg(path.c_str());

        else if (check_ext(path, {"png"}))
            image_ = cimg_library::CImg<img_t>::get_load_png(path.c_str());

        else
            throw std::invalid_argument("Unknown format. Please check again" + path);
        
        img_was_readed_ = true;
    }

    void write_img(const std::string &path) override
    {
        check_created();

        if (check_ext(path, {"jpeg", "jpg"}))
            image_.save_jpeg(path.c_str());

        else if (check_ext(path, {"png"}))
            image_.save_png(path.c_str());

        else
            throw std::invalid_argument("Unknown format. Please check again" + path);
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
        if (!img_was_readed_)
            throw std::runtime_error{"image was not readed"};
    }
};

} // namespace clib
