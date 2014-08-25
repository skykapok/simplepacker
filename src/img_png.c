#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "png.h"

#include "img_png.h"

#define PNG_BYTES_TO_CHECK 4

int
img_loadpng(const char* filename, struct png* png) {
	// open file
	FILE* fp = fopen(filename, "rb");
	if (!fp) { return 0; }

	// init libpng
	png_structp png_ptr;
	png_infop info_ptr;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));

	// check png legality
	int temp;
	char temp2[PNG_BYTES_TO_CHECK];

	temp = fread(temp2, 1, PNG_BYTES_TO_CHECK, fp);
	if (temp < PNG_BYTES_TO_CHECK) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		return 0;
	}

	temp = png_sig_cmp((png_bytep)temp2, (png_size_t)0, PNG_BYTES_TO_CHECK);
	if (temp != 0) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		return 0;
	}

	// read png data
	rewind(fp);
	png_init_io(png_ptr, fp);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

	// get image info
	int w = png_get_image_width(png_ptr, info_ptr);
	int h = png_get_image_height(png_ptr, info_ptr);
	int color_type = png_get_color_type(png_ptr, info_ptr);

	png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

	switch (color_type) {
	case PNG_COLOR_TYPE_RGB_ALPHA:
		png->type = PNG_RGBA;
		png->channel = 4;
		png->buffer = (uint8_t*)malloc(w*h * 4);
		for (int i = 0; i < h; ++i) {
			for (int j = 0; j < w; ++j) {
				int k = (i*w + j) * 4;
				uint8_t alpha = row_pointers[i][j * 4 + 3];
				png->buffer[k + 0] = row_pointers[i][j * 4 + 0] * alpha / 255; // red
				png->buffer[k + 1] = row_pointers[i][j * 4 + 1] * alpha / 255; // green
				png->buffer[k + 2] = row_pointers[i][j * 4 + 2] * alpha / 255; // blue
				png->buffer[k + 3] = alpha; // alpha
			}
		}
		break;

	case PNG_COLOR_TYPE_RGB:
		png->type = PNG_RGB;
		png->channel = 3;
		png->buffer = (uint8_t*)malloc(w*h * 3);
		for (int i = 0; i < h; ++i) {
			for (int j = 0; j < w; ++j) {
				int k = (i*w + j) * 3;
				png->buffer[k + 0] = row_pointers[i][j * 4 + 0]; // red
				png->buffer[k + 1] = row_pointers[i][j * 4 + 1]; // green
				png->buffer[k + 2] = row_pointers[i][j * 4 + 2]; // blue
			}
		}
		break;

	default:
		fclose(fp);
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		return 0;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	png->width = w;
	png->height = h;
	return 1;
}