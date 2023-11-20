
#include <cassert>
#include <cstddef>
#include <cstdio>
extern "C" {
	// libjpeg-dev
	#include <jpeglib.h>
	#include <jerror.h>
	// libpng-dev
	#include <png.h>
}

#include "image.hpp"


uint64_t Pixel::I(void) const {
	return std::max(std::max(r, g), b);
}

std::ostream& operator<<(std::ostream & os, const struct Pixel & p) {
	return os << "(" << p.r << ", " << p.g << ", " << p.b << ")";
}

Image::Image(const std::string & filename) {
	auto check_ext = [](const std::string & s, const std::vector<std::string> exts) -> bool {
		for (auto ext: exts)
			if (s.substr(s.find_last_of(".") + 1) == ext)
				return true;
		return false;
	};
	
	if (check_ext(filename, {"jpeg", "jpg", })) {
		(void) readJPEG(filename);
	} else if (check_ext(filename, {"png", })) {
		(void) readPNG(filename);
	} else {
		std::cout << "Unknown format: " << filename.substr(filename.find_last_of(".") + 1) << std::endl;
		throw 1;
	}
}

Image::Image(size_t height, size_t width) {
	resize(height, width);
}

size_t Image::height(void) const {
	return data.size();
}

size_t Image::width(void) const {
	return data[0].size();
}

void Image::resize(size_t height, size_t width) {
	data.resize(height);
	for (auto& v: data)
		v.resize(width);
}

std::vector<Pixel> & Image::operator[](size_t i) {
	if (i >= height()) {
// 		std::cout << i << "||" << height() << " x " << width() << std::endl;
		assert(i < height());
	}
	return data[i];
}

const std::vector<Pixel> & Image::operator[](size_t i) const {
	if (i >= height()) {
// 		std::cout << i << "||" << height() << " x " << width() << std::endl;
		assert(i < height());
	}
	return data[i];
}

Pixel & Image::at(size_t i, size_t j) {
	if (i >= data.size())
		data.resize(i+1);
	if (j >= data[i].size())
		data[i].resize(j+1);
	return data[i][j];
}

// # # # # # # # # # # # # //
// ## # # # # # # # # # ## //

Image & Image::readJPEG(const std::string & filename) {
	FILE * infile = fopen(filename.c_str(), "rb");
	if (infile == NULL) {
		std::cerr << "Can't open " << filename << std::endl;
		throw 1;
	}
	
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	
	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);
	
	resize(cinfo.output_height, cinfo.output_width);
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);
	
	for (int i = 0; i < cinfo.output_height; ++i) {
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		for (int j = 0; j < cinfo.output_width; ++j) {
			data[i][j] = (struct Pixel) {
				.r = buffer[0][j * 3 + 0],
				.g = buffer[0][j * 3 + 1],
				.b = buffer[0][j * 3 + 2],
			};
		}
	}
	
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	
	fclose(infile);
	
	return *this;
}

void Image::writeJPEG(const std::string & filename) {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	int row_stride;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	FILE * outfile = fopen(filename.c_str(), "wb");
	if (outfile == NULL) {
		std::cerr << "Can't open " << filename << std::endl;
		throw 1;
	}
	
	jpeg_stdio_dest(&cinfo, outfile);
	cinfo.image_width = width();
	cinfo.image_height = height();
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	
	jpeg_set_defaults(&cinfo);
	
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = cinfo.image_width * 3;
	
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, cinfo.image_width * cinfo.input_components, 1);
	
	for (int i = 0; i < cinfo.image_height; ++i) {
		for (int j = 0; j < cinfo.image_width; ++j) {
			buffer[0][j * 3 + 0] = operator[](i)[j].r;
			buffer[0][j * 3 + 1] = operator[](i)[j].g;
			buffer[0][j * 3 + 2] = operator[](i)[j].b;
		}
		row_pointer[0] = buffer[0];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	
	jpeg_finish_compress(&cinfo);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo);
}

// # # # # # # # # # # # # //
// ## # # # # # # # # # ## //

Image & Image::readPNG(const std::string & filename) {
	FILE *fp = fopen(filename.c_str(), "rb");
	
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);

// 	if(setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);
	png_read_info(png, info);

	int      width      = png_get_image_width(png, info);
	int      height     = png_get_image_height(png, info);
	png_byte color_type = png_get_color_type(png, info);
	png_byte bit_depth  = png_get_bit_depth(png, info);

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
	if (color_type == PNG_COLOR_TYPE_RGB ||
		color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png);

	png_read_update_info(png, info);

	png_bytep * row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height);
	for(int i = 0; i < height; ++i) {
		row_pointers[i] = (png_byte *) malloc(png_get_rowbytes(png, info));
	}

	png_read_image(png, row_pointers);
	
	resize(height, width);
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			data[i][j] = {
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
	
	return *this;
}

void Image::writePNG(const std::string & filename) {
	FILE *fp = fopen(filename.c_str(), "wb");

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);

// 	if (setjmp(png_jmpbuf(png))) abort();

	png_init_io(png, fp);

	// Output is 8bit depth, RGBA format.
	png_set_IHDR(
		png,
		info,
		width(), height(),
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);

	// To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
	// Use png_set_filler().
	//png_set_filler(png, 0, PNG_FILLER_AFTER);

	png_bytep * row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * height());
	for(int i = 0; i < height(); ++i) {
		row_pointers[i] = (png_byte *) malloc(png_get_rowbytes(png, info));
		for (int j = 0; j < width(); ++j) {
			row_pointers[i][j * 4 + 0] = data[i][j].r;
			row_pointers[i][j * 4 + 1] = data[i][j].g;
			row_pointers[i][j * 4 + 2] = data[i][j].b;
			row_pointers[i][j * 4 + 3] = 255;
		}
	}

	png_write_image(png, row_pointers);
	png_write_end(png, NULL);

	for (int i = 0; i < height(); ++i) {
		free(row_pointers[i]);
	}
	free(row_pointers);

	fclose(fp);

	png_destroy_write_struct(&png, &info);
}
