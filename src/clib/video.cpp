#include "clib/video.hpp"

namespace clib {

static bool check_ext(const std::string &s, const std::vector<std::string> &exts)
{
    for (auto ext : exts)
        if (s.substr(s.find_last_of(".") + 1) == ext)
            return true;
    return false;
}

cimg_library::CImg<pixel_t> read_video(const std::string &path)
{
    cimg_library::CImg<pixel_t> video;
    if (check_ext(path, {"mp4", "mkv"}))
        video = cimg_library::CImg<pixel_t>::get_load_video(path.c_str());
    else
        throw std::invalid_argument("Unknown format: " + path);
    return video;
}

void write_video(const cimg_library::CImg<pixel_t> &video, const std::string &path)
{
    if (check_ext(path, {"mp4", "mkv"}))
        video.save_video(path.c_str());
    else
        throw std::invalid_argument("Unknown format: " + path);
}

} // namespace clib