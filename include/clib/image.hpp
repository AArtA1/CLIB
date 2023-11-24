#pragma once

#include "common.hpp"
#include <iostream>
#include "X11/Xlib.h"
#include "CImg.h"

using namespace cimg_library;


template<typename T>
CImg<T> read(std::string& path);

template<typename T>
void write(const CImg<T>& image,const std::string& path);

