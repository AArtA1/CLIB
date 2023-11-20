#pragma once

#include <iostream>
#include <vector>
#include <cstddef>


struct Pixel {
	uint64_t r, g, b;
	
	uint64_t I(void) const;
	
	friend std::ostream& operator<<(std::ostream & os, const Pixel & p);
};


class Image {
	using _pixel = Pixel;
	
	std::vector<std::vector<Pixel>> data;
	
public:
	Image(const std::string & filename);
	Image(size_t height, size_t width);
	
	Image & readJPEG(const std::string & filename);
	void writeJPEG(const std::string & filename);
	
	Image & readPNG(const std::string & filename);
	void writePNG(const std::string & filename);
	
	size_t height(void) const;
	size_t width(void) const;
	void resize(size_t height, size_t width);
	
	std::vector<Pixel> & operator[](size_t i);
	const std::vector<Pixel> & operator[](size_t i) const;
	Pixel & at(size_t i, size_t j);
};
