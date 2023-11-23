#include "clib/image.hpp"

template<typename T>
CImg<T> read(std::string& path){
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
        //CLOG(error) << "Unknown format: " << path.substr(path.find_last_of(".") + 1) << std::endl;
#endif
        throw "Unknown format. Please check again";
    }
    return image;
}

template<typename T>
void write(const CImg<T>& image,const std::string& path){
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
        //CLOG(error) << "Unknown format: " << path.substr(path.find_last_of(".") + 1) << std::endl;
#endif
        throw "Unknown format. Please check again";
    }
}