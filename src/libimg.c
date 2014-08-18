#include "lualib.h"
#include "lauxlib.h"
#include "png.h"

#define PNG_BYTES_TO_CHECK 4

static int
lloadpng(lua_State *L) {
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep* row_pointers;
	
	int x, y, temp;
	char temp2[PNG_BYTES_TO_CHECK];

	int w, h, color_type;
	unsigned char* buffer;

	fp = fopen(lua_tostring(L, -1), "rb");
	if (fp == NULL) {
		return 0;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));

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

	rewind(fp);
	png_init_io(png_ptr, fp);
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);
	color_type = png_get_color_type(png_ptr, info_ptr);
	w = png_get_image_width(png_ptr, info_ptr);
	h = png_get_image_height(png_ptr, info_ptr);
	row_pointers = png_get_rows(png_ptr, info_ptr);

	switch (color_type) {
		case PNG_COLOR_TYPE_RGB_ALPHA:
			buffer = (unsigned char*)lua_newuserdata(L, w * h * 32);
			for (y = 0; y<h; ++y) {
				for (x = 0; x<w * 4;) {
					buffer[y*w + x] = row_pointers[y][x++]; // red  
					buffer[y*w + x] = row_pointers[y][x++]; // green  
					buffer[y*w + x] = row_pointers[y][x++]; // blue  
					buffer[y*w + x] = row_pointers[y][x++]; // alpha  
				}
			}
			break;

		case PNG_COLOR_TYPE_RGB:
			buffer = (unsigned char*)lua_newuserdata(L, w * h * 24);
			for (y = 0; y<h; ++y) {
				for (x = 0; x<w * 3;) {
					buffer[y*w + x] = row_pointers[y][x++]; // red  
					buffer[y*w + x] = row_pointers[y][x++]; // green  
					buffer[y*w + x] = row_pointers[y][x++]; // blue  
				}
			}
			break;

		default:
			fclose(fp);
			png_destroy_read_struct(&png_ptr, &info_ptr, 0);
			return 0;
	}
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	lua_pushnumber(L, color_type);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 4;
}

int
register_libimage(lua_State *L) {
	luaL_Reg l[] = {
		{ "loadpng", lloadpng },
		{ NULL, NULL }
	};

	luaL_newlib(L, l);
	return 1;
}