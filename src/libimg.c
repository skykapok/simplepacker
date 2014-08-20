#include <stdlib.h>
#include <string.h>

#include "lualib.h"
#include "lauxlib.h"
#include "png.h"

#define PNG_BYTES_TO_CHECK 4

static int
lloadpng(lua_State *L) {
	// open file
	const char* path = luaL_checkstring(L, -1);
	FILE* fp = fopen(path, "rb");
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
	unsigned char* buf = NULL;

	switch (color_type) {
		case PNG_COLOR_TYPE_RGB_ALPHA:
			buf = (unsigned char*)lua_newuserdata(L, w*h * 4);
			for (int i = 0; i < h; ++i) {
				for (int j = 0; j < w; ++j) {
					int k = (i*w + j) * 4;
					buf[k + 0] = row_pointers[i][j * 4 + 0]; // red  
					buf[k + 1] = row_pointers[i][j * 4 + 1]; // green  
					buf[k + 2] = row_pointers[i][j * 4 + 2]; // blue  
					buf[k + 3] = row_pointers[i][j * 4 + 3]; // alpha  
				}
			}
			lua_pushstring(L, "RGBA");
			break;

		case PNG_COLOR_TYPE_RGB:
			buf = (unsigned char*)lua_newuserdata(L, w*h * 3);
			for (int i = 0; i < h; ++i) {
				for (int j = 0; j < w; ++j) {
					int k = (i*w + j) * 3;
					buf[k + 0] = row_pointers[i][j * 4 + 0]; // red  
					buf[k + 1] = row_pointers[i][j * 4 + 1]; // green  
					buf[k + 2] = row_pointers[i][j * 4 + 2]; // blue  
				}
			}
			lua_pushstring(L, "RGB");
			break;

		default:
			fclose(fp);
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);
			return 0;
	}

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 4;  // buf, color_type, w, h
}

static int
lsaveppm(lua_State *L) {
	// check args
	const char* path = luaL_checkstring(L, -5);  // filename without extension
	int w = luaL_checkint(L, -4);
	int h = luaL_checkint(L, -3);
	const char* color_type = luaL_checkstring(L, -2);  // RGBA or RGB
	unsigned char* buf = lua_touserdata(L, -1);
	int has_a = strcmp(color_type, "RGBA") == 0;

	char fn[512];
	FILE *fp;

	// open file for rgb channel
	sprintf(fn, "%s.ppm", path);
	fp = fopen(fn, "wb");
	if (!fp) { luaL_error(L, "can't write to %s", fn); }

	// ppm header
	fprintf(fp, "P6\n%d %d\n255\n", w, h);

	// write rgb
	unsigned char* rgb = (unsigned char*)malloc(w*h * 3);
	for (int i = 0; i < w*h; ++i) {
		if (has_a && buf[i * 4 + 3] == 0) {  // write black if alpha is 0
			rgb[i * 3 + 0] = 0;
			rgb[i * 3 + 1] = 0;
			rgb[i * 3 + 2] = 0;
		} else {
			rgb[i * 3 + 0] = buf[i * 4 + 0];
			rgb[i * 3 + 1] = buf[i * 4 + 1];
			rgb[i * 3 + 2] = buf[i * 4 + 2];
		}
	}
	fwrite(rgb, 3, w*h, fp);
	fclose(fp);
	free(rgb);
	
	if (has_a) {
		// open files for alpha channel
		sprintf(fn, "%s.pgm", path);
		fp = fopen(fn, "wb");
		if (fp == NULL) { luaL_error(L, "can't write to %s", fn); }

		// pgm header
		fprintf(fp, "P5\n%d %d\n255\n", w, h);

		// write alpha
		unsigned char* alpha = (unsigned char*)malloc(w*h);
		for (int i = 0; i < w*h; ++i) {
			alpha[i] = buf[i * 4 + 3];
		}
		fwrite(alpha, 1, w*h, fp);
		fclose(fp);
		free(alpha);
	}

	return 0;
}

int
register_libimage(lua_State *L) {
	luaL_Reg l[] = {
		{ "loadpng", lloadpng },
		{ "saveppm", lsaveppm },
		{ NULL, NULL }
	};

	luaL_newlib(L, l);
	return 1;
}