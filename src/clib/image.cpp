#include <cassert>
#include <cstddef>
#include <cstdio>
extern "C"
{
    // libjpeg-dev
#include <jerror.h>
#include <jpeglib.h>
    // libpng-dev
#include <png.h>
}

#include "../../include/clib/image.hpp"
#include "../../include/clib/logs.hpp"

namespace clib
{

Image read(const std::string &path)
{
    auto check_ext = [](const std::string &s, const std::vector<std::string> exts) -> bool {
        for (auto ext : exts)
            if (s.substr(s.find_last_of(".") + 1) == ext)
                return true;
        return false;
    };

    Image image;

    if (check_ext(path, {
                            "jpeg",
                            "jpg",
                        }))
    {
        image = Image::read_JPEG(path);
    }
    else if (check_ext(path, {
                                 "png",
                             }))
    {
        image = Image::read_PNG(path);
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

void write(const Image &image, const std::string &path)
{
    auto check_ext = [](const std::string &s, const std::vector<std::string> exts) -> bool {
        for (auto ext : exts)
            if (s.substr(s.find_last_of(".") + 1) == ext)
                return true;
        return false;
    };

    if (check_ext(path, {
                            "jpeg",
                            "jpg",
                        }))
    {
        Image::write_JPEG(image, path);
    }
    else if (check_ext(path, {
                                 "png",
                             }))
    {
        Image::write_PNG(image, path);
    }
    else
    {
#ifndef NDEBUG
        CLOG(error) << "Unknown format: " << path.substr(path.find_last_of(".") + 1) << std::endl;
#endif
        throw "Unknown format. Please check again";
    }
}

uint64_t Pixel::I(void) const
{
    return std::max(std::max(r, g), b);
}

std::ostream &operator<<(std::ostream &os, const struct Pixel &p)
{
    return os << "(" << p.r << ", " << p.g << ", " << p.b << ")";
}

size_t Image::get_height() const
{
    return data.size();
}

size_t Image::get_width() const
{
    return data[0].size();
}

void Image::resize(size_t height, size_t width)
{
    data.resize(height);
    for (auto &v : data)
        v.resize(width);
}

std::vector<Pixel> &Image::operator[](size_t i)
{
    if (i >= get_height())
    {
        // 		std::cout << i << "||" << height() << " x " << width() << std::endl;
        assert(i < get_height());
    }
    return data[i];
}

const std::vector<Pixel> &Image::operator[](size_t i) const
{
    if (i >= get_height())
    {
        // 		std::cout << i << "||" << height() << " x " << width() << std::endl;
        assert(i < get_height());
    }
    return data[i];
}

Pixel &Image::at(size_t i, size_t j)
{
    if (i >= data.size())
        data.resize(i + 1);
    if (j >= data[i].size())
        data[i].resize(j + 1);
    return data[i][j];
}

// # # # # # # # # # # # # //
// ## # # # # # # # # # ## //

Image Image::read_JPEG(const std::string &path)
{
#ifndef NDEBUG
    CLOG(trace) << "Reading JPEG image";
#endif

    FILE *infile = fopen(path.c_str(), "rb");
    if (infile == NULL)
    {
#ifndef NDEBUG
        CLOG(error) << "Can't open";
#endif
        throw "Can't ride file. Check your filename path";
    }

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);

    (void)jpeg_read_header(&cinfo, TRUE);
    (void)jpeg_start_decompress(&cinfo);

    Image image;

    image.resize(cinfo.output_height, cinfo.output_width);

    JSAMPARRAY buffer =
        (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);

    for (int i = 0; i < cinfo.output_height; ++i)
    {
        (void)jpeg_read_scanlines(&cinfo, buffer, 1);
        for (int j = 0; j < cinfo.output_width; ++j)
        {
            image.data[i][j] = (struct Pixel){
                .r = buffer[0][j * 3 + 0],
                .g = buffer[0][j * 3 + 1],
                .b = buffer[0][j * 3 + 2],
            };
        }
    }

    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(infile);

    return image;
}

void Image::write_JPEG(const Image &image, const std::string &path)
{
#ifndef NDEBUG
    CLOG(trace) << "Writing JPEG image";
#endif
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    FILE *outfile = fopen(path.c_str(), "wb");
    if (outfile == NULL)
    {
#ifndef NDEBUG
        CLOG(error) << "Can't write";
#endif
        throw "Write has not been written. Check your filename path";
    }

    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = image.get_width();
    cinfo.image_height = image.get_height();
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);

    jpeg_start_compress(&cinfo, TRUE);
    row_stride = cinfo.image_width * 3;

    JSAMPARRAY buffer =
        (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.image_width * cinfo.input_components, 1);

    for (int i = 0; i < cinfo.image_height; ++i)
    {
        for (int j = 0; j < cinfo.image_width; ++j)
        {
            buffer[0][j * 3 + 0] = image.data[i][j].r;
            buffer[0][j * 3 + 1] = image.data[i][j].g;
            buffer[0][j * 3 + 2] = image.data[i][j].b;
        }
        row_pointer[0] = buffer[0];
        (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
}

// # # # # # # # # # # # # //
// ## # # # # # # # # # ## //

Image Image::read_PNG(const std::string &path)
{
#ifndef NDEBUG
    CLOG(trace) << "Reading PNG image";
#endif
    FILE *fp = fopen(path.c_str(), "rb");

    if (fp == nullptr)
    {
#ifndef NDEBUG
        CLOG(error) << "Can't read the file";
#endif
        throw "File can't be read. Check your filename path";
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);

    // 	if(setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);
    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (bit_depth == 16)
        png_set_strip_16(png);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);

    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    // These color_type don't have an alpha channel then fill it with 0xff.
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int i = 0; i < height; ++i)
    {
        row_pointers[i] = (png_byte *)malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, row_pointers);

    Image image;

    image.resize(height, width);

    for (int i = 0; i < height; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            image.data[i][j] = {
                .r = row_pointers[i][j * 4 + 0],
                .g = row_pointers[i][j * 4 + 1],
                .b = row_pointers[i][j * 4 + 2],
            };
        }
        free(row_pointers[i]);
    }

    free(row_pointers);

    fclose(fp);

    png_destroy_read_struct(&png, &info, NULL);

    return image;
}

void Image::write_PNG(const Image &image, const std::string &path)
{
#ifndef NDEBUG
    CLOG(trace) << "Writing JPEG image";
#endif

    FILE *fp = fopen(path.c_str(), "wb");

    if (fp == nullptr)
    {
#ifndef NDEBUG
        CLOG(error) << "Can't read the file";
#endif
        throw "File can't be read. Check your filename path";
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);

    // 	if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    // Output is 8bit depth, RGBA format.
    png_set_IHDR(png, info, image.get_width(), image.get_height(), 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    // png_set_filler(png, 0, PNG_FILLER_AFTER);

    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * image.get_height());
    for (int i = 0; i < image.get_height(); ++i)
    {
        row_pointers[i] = (png_byte *)malloc(png_get_rowbytes(png, info));
        for (int j = 0; j < image.get_width(); ++j)
        {
            row_pointers[i][j * 4 + 0] = image.data[i][j].r;
            row_pointers[i][j * 4 + 1] = image.data[i][j].g;
            row_pointers[i][j * 4 + 2] = image.data[i][j].b;
            row_pointers[i][j * 4 + 3] = 255;
        }
    }

    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

    for (int i = 0; i < image.get_height(); ++i)
    {
        free(row_pointers[i]);
    }
    free(row_pointers);

    fclose(fp);

    png_destroy_write_struct(&png, &info);
}

} // namespace clib