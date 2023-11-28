#include "clib/image.hpp"

static bool check_ext(const std::string &s, const std::vector<std::string> &exts)
{
    for (auto ext : exts)
        if (s.substr(s.find_last_of(".") + 1) == ext)
            return true;
    return false;
}

namespace clib
{

template <typename T> CImg<T> read(const std::string &path)
{
    CImg<T> image;
    if (check_ext(path, {
                            "jpeg",
                            "jpg",
                        }))
    {

        image = CImg<T>::get_load_jpeg(path.c_str());
    }
    else if (check_ext(path, {
                                 "png",
                             }))
    {
        image = CImg<T>::get_load_png(path.c_str());
    }
    else
    {
#ifndef NDEBUG
        CLOG(error) << "Unknown format: " << path.substr(path.find_last_of(".") + 1) << std::endl;
#endif
        throw "Unknown format. Please check again";
    }
    return image;
}

template <typename T> void write(const CImg<T> &image, const std::string &path)
{
    if (check_ext(path, {
                            "jpeg",
                            "jpg",
                        }))
    {
        image.save_jpeg(path.c_str());
    }
    else if (check_ext(path, {
                                 "png",
                             }))
    {
        image.save_png(path.c_str());
    }
    else
    {
#ifndef NDEBUG
        CLOG(error) << "Unknown format: " << path.substr(path.find_last_of(".") + 1) << std::endl;
#endif
        throw "Unknown format. Please check again";
    }
}

template CImg<unsigned int> read(const std::string &path);

template CImg<float> read(const std::string &path);

template void write<unsigned int>(const CImg<unsigned int> &image, const std::string &path);

template void write<float>(const CImg<float> &image, const std::string &path);

} // namespace clib
