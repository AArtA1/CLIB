#include "clib/ImgView.hpp"
#include <cassert>
#include <limits>

namespace clib
{

using idx_t = ImgView::idx_t;
using pixel_t = ImgView::pixel_t;

void CImgView::init(idx_t rows, idx_t cols, idx_t clrs)
{
    assert(rows <= std::numeric_limits<unsigned>::max());
    assert(cols <= std::numeric_limits<unsigned>::max());
    assert(clrs <= std::numeric_limits<unsigned>::max());
    image_ = cimg_library::CImg<pixel_t>(static_cast<unsigned>(cols), static_cast<unsigned>(rows), 1,
                                         static_cast<unsigned>(clrs));
    img_created_ = true;
}

idx_t CImgView::rows() const
{
    check_created();
    return static_cast<idx_t>(image_.height());
}
idx_t CImgView::cols() const
{
    check_created();
    return static_cast<idx_t>(image_.width());
}
idx_t CImgView::clrs() const
{
    check_created();
    return static_cast<idx_t>(image_.spectrum());
}

// CImg stores data as [width,height]. Therefore, the data in cimg are transposed
pixel_t CImgView::get(idx_t i, idx_t j, idx_t clr) const
{
    assert(image_.depth() == 1);
    assert(i < rows());
    assert(j < cols());
    assert(clr < clrs());

    check_created();
    return image_(static_cast<unsigned>(j), static_cast<unsigned>(i), 0, static_cast<unsigned>(clr));
}

// CImg stores data as [width,height]. Therefore, the data in cimg are transposed
void CImgView::set(pixel_t val, idx_t i, idx_t j, idx_t clr)
{
    assert(image_.depth() == 1);
    assert(i < rows());
    assert(j < cols());
    assert(clr < clrs());

    check_created();
    image_(static_cast<unsigned>(j), static_cast<unsigned>(i), 0, static_cast<unsigned>(clr)) = val;
}

void CImgView::read_img(const std::string &path)
{
    if (check_ext(path, {"jpeg", "jpg"}))
        image_ = cimg_library::CImg<pixel_t>::get_load_jpeg(path.c_str());

    else if (check_ext(path, {"png"}))
        image_ = cimg_library::CImg<pixel_t>::get_load_png(path.c_str());

    else
        throw std::invalid_argument("Unknown format. Please check again" + path);

    img_created_ = true;
}

void CImgView::write_img(const std::string &path)
{
    check_created();

    if (check_ext(path, {"jpeg", "jpg"}))
        image_.save_jpeg(path.c_str());

    else if (check_ext(path, {"png"}))
        image_.save_png(path.c_str());

    else
        throw std::invalid_argument("Unknown format. Please check again" + path);
}
bool CImgView::check_ext(const std::string &s, const std::vector<std::string> &exts)
{
    for (auto ext : exts)
        if (s.substr(s.find_last_of(".") + 1) == ext)
            return true;
    return false;
}

void CImgView::check_created() const
{
    if (!img_created_)
        throw std::runtime_error{"image was not readed"};
}

} // namespace clib
