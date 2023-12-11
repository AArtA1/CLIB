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
 * \brief Обёртка над классом для работы с изображениями
 *
 * \details Абстрагирует любую библиотеку в набор методов
 */
struct ImgView
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

    virtual idx_t rows() const = 0; // height
    virtual idx_t cols() const = 0; // width
    virtual idx_t clrs() const = 0; // количество цветов

    // Работа с пикселями
    virtual pixel_t get(idx_t i, idx_t j, idx_t clr) const = 0;
    virtual void set(pixel_t val, idx_t i, idx_t j, idx_t clr) = 0;

    virtual void read_img(const std::string &path) = 0;
    virtual void write_img(const std::string &path) = 0;

    virtual ~ImgView()
    {
    }
};

struct CImgView : ImgView
{

  private:
    cimg_library::CImg<pixel_t> image_;
    bool img_was_readed_ = false;

  public:
    // TODO virtual?????
    void init(idx_t rows, idx_t cols, idx_t clrs)
    {
        image_ = cimg_library::CImg<pixel_t>(cols, rows, 1, clrs);
        img_was_readed_ = true;
    }

    // TODO затычка для работа с видео
    void load(const cimg_library::CImg<pixel_t> &image)
    {
        image_ = image;
        img_was_readed_ = true;
    }
    const cimg_library::CImg<pixel_t> & unload()
    {
        return image_;
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
    idx_t clrs() const override
    {
        check_created();
        return static_cast<idx_t>(image_.spectrum());
    }

    // CImg stores data as [width,height]. Therefore, the data in cimg are transposed
    pixel_t get(idx_t i, idx_t j, idx_t clr) const override
    {
        assert(i < static_cast<idx_t>(image_.height()));
        assert(j < static_cast<idx_t>(image_.width()));
        assert(clr < static_cast<idx_t>(image_.spectrum()));

        check_created();
        return image_(j, i, 0, clr);
    }

    // CImg stores data as [width,height]. Therefore, the data in cimg are transposed
    void set(pixel_t val, idx_t i, idx_t j, idx_t clr) override
    {
        assert(i < static_cast<idx_t>(image_.height()));
        assert(j < static_cast<idx_t>(image_.width()));
        assert(clr < static_cast<idx_t>(image_.spectrum()));

        check_created();
        image_(j, i, 0, clr) = val;
    }

    void read_img(const std::string &path) override
    {
        if (check_ext(path, {"jpeg", "jpg"}))
            image_ = cimg_library::CImg<pixel_t>::get_load_jpeg(path.c_str());

        else if (check_ext(path, {"png"}))
            image_ = cimg_library::CImg<pixel_t>::get_load_png(path.c_str());

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
    static bool check_ext(const std::string &s, const std::vector<std::string> &exts)
    {
        for (auto ext : exts)
            if (s.substr(s.find_last_of(".") + 1) == ext)
                return true;
        return false;
    }
private:
    void check_created() const
    {
        if (!img_was_readed_)
            throw std::runtime_error{"image was not readed"};
    }
};

} // namespace clib
