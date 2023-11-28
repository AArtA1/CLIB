#pragma once

#include "CImg.h"
#include "X11/Xlib.h"
#include "common.hpp"
#include <iostream>

using namespace cimg_library;

namespace clib
{

template <typename T> CImg<T> read(const std::string &path);

template <typename T> void write(const CImg<T> &image, const std::string &path);

} // namespace clib
