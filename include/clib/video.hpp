#pragma once

#include "video.hpp"

namespace clib
{

// ----------------------- SUPPORTS ONLY MP4, MKV extensions --------------------

template <typename T> cimg_library::CImg<T> read_video(const std::string &path)
{
    cimg_library::CImg<T> video;
    if (check_ext(path, {
                            "mp4",
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


template<typename T> class video
{
    std::vector<img_rgb<T>> frames;

  public:


    video() = default;

    video(const T& prototype, const std::string & video_path , size_t req_threads = 0){
        cimg_library::CImg img_flt = read_video<IMG_T>(video_path);
        frames.reserve(img_flt.depth());
        for(size_t i = 0; i < img_flt.depth();++i){
            frames.push_back(img_rgb(prototype,img_flt,i,req_threads));
        }
    }

    video(std::vector<img_rgb<T>> frames_) : frames(frames_){
    }

    void write(const std::string& video_path){
        cimg_library::CImg<IMG_T> img_flt(cols(), rows(), frames.size(), 3);

        for(size_t i = 0; i < depth();++i){
            frames[i].write(img_flt,i);
        }

        write_video(img_flt,video_path);
    }

    // rows - height of image
    const size_t rows() const
    {
        return frames[0].rows();
    }

    // cols - width of image
    const size_t cols() const
    {
        return frames[0].cols();
    }
    
    const size_t depth() const
    {
        return frames.size();
    }

    const img_rgb<T>& operator()(const size_t i) const{
        assert(i >= 0 && i < frames.size());

        return frames[i];
    }


};








} // namespace clib