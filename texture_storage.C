#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <exception>
#include "rgba.h"
#include "texture_storage.h"
#include <iostream>

namespace texture {
	using namespace model;
	using namespace std;
	void PngLoader::init() {
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 
						 (png_voidp)0, 0, 0);
		if (png_ptr) {
			info_ptr = png_create_info_struct(png_ptr);
		}
		
		if (!png_ptr || !info_ptr) {
			destroy();
			throw exception();
		}
	}

	bool PngLoader::open() {
		fImage = fopen(fileName,"rb");
		if (fImage) {
			png_init_io(png_ptr, fImage);
			png_read_png(png_ptr, info_ptr, 
				     PNG_TRANSFORM_PACKING|
				     PNG_TRANSFORM_EXPAND|
				     PNG_TRANSFORM_STRIP_16, 
				     NULL);
			width = info_ptr->width;
			height = info_ptr->height;
		}
		return fImage;
	}

	void PngLoader::getPixels(RGBA* pixels) {
		if (width && height && pixels) {
			memset(pixels, 0, sizeof(RGBA)*width*height);
			
			png_bytep *row_pointers = (png_bytep*)png_malloc(png_ptr, height*sizeof(png_bytep));
			memset(pixels, 0xff, width*height*sizeof(RGBA));
			int i;
			row_pointers = png_get_rows(png_ptr, info_ptr);
			for (i=0; i<height; i++) 
				memcpy((png_bytep)(pixels+i*width), row_pointers[i], width*sizeof(RGBA));
			
		}
	}

	RGBA* PngLoader::getPixelsRGBA() {
		if (!pixels) {
			pixels=new RGBA[width*height];
			getPixels(pixels);
		}
		return pixels;
	}
	
	void PngLoader::destroy() {
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		png_ptr=0;
		info_ptr=0;
		end_info=0;
	}

	PngLoader::PngLoader(char * filename) {
		fileName=strdup(filename);
		png_ptr=0;
		info_ptr=0;
		end_info=0;
		width=0;
		height=0;
		pixels=0;
		fImage=0;
		init();
		open();
	}

	PngLoader::~PngLoader() {
		free(fileName);
		if (pixels) delete[] pixels;
		destroy();
		if (fImage) fclose(fImage);
	}
};
