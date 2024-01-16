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

const std::vector<std::string> image_extensions = {"jpg","jpeg","png"};


/*!
 * \brief Обёртка над классом для работы с изображениями
 *
 * \details Абстрагирует любую библиотеку в набор методов
 */
struct ImgView
{
    using pixel_t = int;
    using idx_t = size_t;

    // colors number
    enum spectrum
    {
        R = 0,
        G = 1,
        B = 2
    };

    virtual void init(idx_t cols, idx_t rows, idx_t colors) = 0;

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
    bool img_created_ = false;

public:
    void init(idx_t rows, idx_t cols, idx_t colors) override;
    
    idx_t rows() const override;
    
    idx_t cols() const override;

    idx_t clrs() const override;

    pixel_t get(idx_t i, idx_t j, idx_t clr) const override;

    // CImg stores data as [width,height]. Therefore, the data in cimg are transposed
    void set(pixel_t val, idx_t i, idx_t j, idx_t clr) override;

    void read_img(const std::string &path) override;

    void write_img(const std::string &path) override;
    
    static bool check_ext(const std::string &s, const std::vector<std::string> &exts);
  
private:

    void check_created() const;
};
} // namespace clib
