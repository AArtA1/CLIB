#pragma once

#include <stdint.h>
#include <iostream>
#include <vector>

namespace clib {

struct Pixel
{
    uint64_t r, g, b;
    uint64_t I(void) const;

    friend std::ostream &operator<<(std::ostream &os, const Pixel &p);
};

class Image
{
    std::vector<std::vector<Pixel>> data;

  public:
    Image() = default;
    Image(const std::string &path);

    static Image read_JPEG(const std::string &path);
    static void write_JPEG(const Image &image, const std::string &path);

    static Image read_PNG(const std::string &path);
    static void write_PNG(const Image &image, const std::string &path);

    size_t get_height() const;
    size_t get_width() const;
    void resize(size_t height, size_t width);

    std::vector<Pixel> &operator[](size_t i);
    const std::vector<Pixel> &operator[](size_t i) const;
    Pixel &at(size_t i, size_t j);
};

} // namespace clib