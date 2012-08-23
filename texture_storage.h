#ifndef __TEXTURE_STORAGE_H
#define __TEXTURE_STORAGE_H

#include <png.h>
#include "rgba.h"

namespace texture {
	using namespace model;
	class PngLoader {
		png_structp png_ptr;
		png_infop info_ptr;
		png_infop end_info;
		FILE * fImage;
		char * fileName;
		png_uint_32 width;
		png_uint_32 height;
		RGBA* pixels;
	public:
		PngLoader(char * filename);
		~PngLoader();
		bool isPng();
		void loadPixels();
		bool open();
		void close();
		void init();
		void destroy();
		void getPixels(model::RGBA* pixels);
		RGBA *getPixelsRGBA();
		inline int getHeight() {return height;};
		inline int getWidth() {return width;};
	};
};

#endif
