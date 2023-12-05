#pragma once

#include "video.hpp"

namespace clib
{

// ----------------------- SUPPORTS ONLY MP4, MKV extensions --------------------

template <typename T> cimg_library::CImg<T> read_video(const std::string &path)
{
    cimg_library::CImg<T> video;
    if (check_ext(path, {
                            "mk4",
                            "mkv"
                        }))
    {

        video = cimg_library::CImg<T>::get_load_video(path.c_str());
    }
    else
    {
#ifndef NDEBUG
        CLOG(error) << "Unknown format: " << path.substr(path.find_last_of(".") + 1) << std::endl;
#endif
        throw "Unknown format. Please check again";
    }
    return video;
}

template <typename T> void write_video(const cimg_library::CImg<T> &video, const std::string &path)
{
    if (check_ext(path, {
                            "mk4",
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
        throw "Unknown format. Please check again";
    }
}


// -----------------------------------------------------------------------




// template<typename T> class video
// {
//     std::vector<img_rgb<T>> frames;

//   public:
//     // img_rgb(const T &prototype, const std::string &img_path, size_t req_threads = 0)
//     // {
//     //     cimg_library::CImg img_flt = read_video<IMG_T>(img_path);

//     //     assert(img_flt.spectrum() == 3);

//     //     frames.resize(img_flt.depth());
        
//     //     for(size_t i = 0; i < frames.size();++i){
//     //         frames[i] = img_flt()
//     //     }

//     // }


//     // void write(const std::string &out_path)
//     // {
//     //     cimg_library::CImg<IMG_T> img_flt(r.cols(), r.rows(), 1, 3);

//     //     r.write(img_flt, depth - 1, R);
//     //     g.write(img_flt, depth - 1, G);
//     //     b.write(img_flt, depth - 1, B);

//     //     clib::write_img(img_flt, out_path);
//     // }

//     // rows - height of image
//     const size_t rows() const
//     {
//         return depth[0].rows();
//     }

//     // cols - width of image
//     const size_t cols() const
//     {
//         return depth[0].cols();
//     }
// };








} // namespace clib