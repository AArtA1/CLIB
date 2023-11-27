#pragma once

#include "common.hpp"
#include <iostream>
#include "X11/Xlib.h"
#include "CImg.h"


using namespace cimg_library;


namespace clib{

template<typename T>
CImg<T> read(const std::string& path);

template<typename T>
void write(const CImg<T>& image,const std::string& path);


} // namespace clib
