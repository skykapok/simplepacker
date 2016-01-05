#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lualib.h"
#include "lauxlib.h"

#include "img_png.h"
#include "img_ppm.h"

#define PIXEL_FORMAT_UNKNOW 0
#define PIXEL_FORMAT_RGB 1
#define PIXEL_FORMAT_RGBA 2

static int
_check_pixel_format(const char* fmt) {
	if (!fmt) { return PIXEL_FORMAT_UNKNOW; }

	if (strcmp(fmt, "RGB") == 0) {
		return PIXEL_FORMAT_RGB;
	} else if (strcmp(fmt, "RGBA") == 0) {
		return PIXEL_FORMAT_RGBA;
	} else {
		return PIXEL_FORMAT_UNKNOW;
	}
}

static void
_blit(uint8_t* src, uint8_t* dst, int ks, int kd, int depth) {
	for (int i = 0; i < depth; ++i) {
		dst[kd + i] = src[ks + i];
	}
}

static int
lloadpng(lua_State *L) {
	const char* fn = luaL_checkstring(L, -1);

	struct png png;
	int ok = img_loadpng(fn, &png);

	if (!ok) {
		return 0;
	}

	int sz = png.width * png.height * png.channel;
	uint8_t* buf = lua_newuserdata(L, sz);
	memcpy(buf, png.buffer, sz);
	free(png.buffer);

	switch (png.type)
	{
	case PNG_RGBA:
		lua_pushstring(L, "RGBA");
		break;

	case PNG_RGB:
		lua_pushstring(L, "RGB");
		break;

	default:
		break;
	}

	lua_pushnumber(L, png.width);
	lua_pushnumber(L, png.height);

	return 4;  // buf, color_type, w, h
}

static int
lsavepng(lua_State *L) {
	const char* fn = luaL_checkstring(L, -5);  // filename without extension
	int w = luaL_checkint(L, -4);
	int h = luaL_checkint(L, -3);
	int pixfmt = _check_pixel_format(luaL_checkstring(L, -2));
	uint8_t* buf = (uint8_t*)lua_touserdata(L, -1);

	int ok = 0;
	if (pixfmt == PIXEL_FORMAT_RGB) {
		ok = img_savepng(fn, PNG_RGB, w, h, buf);
	} else if (pixfmt == PIXEL_FORMAT_RGBA) {
		ok = img_savepng(fn, PNG_RGBA, w, h, buf);
	}

	if (!ok) {
		luaL_error(L, "save png failed");
	}

	return 0;
}

static int
lloadppm(lua_State *L) {
	const char* fn = luaL_checkstring(L, -1);  // filename without extension

	struct ppm ppm;
	int ok = img_loadppm(fn, &ppm);

	if (!ok) {
		return 0;
	}

	int sz = ppm.width * ppm.height * ppm.step;
	uint8_t* buf = lua_newuserdata(L, sz);
	memcpy(buf, ppm.buffer, sz);
	free(ppm.buffer);

	switch (ppm.type)
	{
	case PPM_RGBA8:
		lua_pushstring(L, "RGBA");
		break;

	case PPM_RGB8:
		lua_pushstring(L, "RGB");
		break;

	default:
		lua_pop(L, 1);
		return 0;
	}

	lua_pushnumber(L, ppm.width);
	lua_pushnumber(L, ppm.height);

	return 4;
}

static int
lsaveppm(lua_State *L) {
	const char* fn = luaL_checkstring(L, -5);  // filename without extension
	int w = luaL_checkint(L, -4);
	int h = luaL_checkint(L, -3);
	int pixfmt = _check_pixel_format(luaL_checkstring(L, -2));
	uint8_t* buf = (uint8_t*)lua_touserdata(L, -1);

	int ok = 0;
	if (pixfmt == PIXEL_FORMAT_RGB) {
		ok = img_saveppm(fn, PPM_RGB8, w, h, buf);
	} else if (pixfmt == PIXEL_FORMAT_RGBA) {
		ok = img_saveppm(fn, PPM_RGBA8, w, h, buf);
	}

	if (!ok) {
		luaL_error(L, "save ppm failed");
	}

	return 0;
}

static int
lnewimg(lua_State *L) {
	int s = luaL_checkint(L, -2);
	int pixfmt = _check_pixel_format(luaL_checkstring(L, -1));
	uint8_t* buf = NULL;
	int buf_size = 0;

	switch (pixfmt) {
	case PIXEL_FORMAT_RGB:
		buf_size = s*s * 3;
		buf = lua_newuserdata(L, buf_size);
		memset(buf, 0, buf_size);
		break;

	case PIXEL_FORMAT_RGBA:
		buf_size = s*s * 4;
		buf = lua_newuserdata(L, buf_size);
		memset(buf, 0, buf_size);
		break;

	default:
		luaL_error(L, "unknown pixel format");
	}

	return 1;
}

static int
lblitimg(lua_State *L) {
	// parse arguments
	int y = luaL_checkint(L, -1);
	int x = luaL_checkint(L, -2);
	int src_h = luaL_checkint(L, -3);
	int src_w = luaL_checkint(L, -4);
	uint8_t* src = lua_touserdata(L, -5);
	int pixfmt = _check_pixel_format(luaL_checkstring(L, -6));
	int dst_s = luaL_checkint(L, -7);
	uint8_t* dst = lua_touserdata(L, -8);

	int ks, kd, depth;

	switch (pixfmt) {  // copy pixels
	case PIXEL_FORMAT_RGB:
		depth = 3;
		break;

	case PIXEL_FORMAT_RGBA:
		depth = 4;
		break;

	default:
		luaL_error(L, "unknown pixel format");
		break;
	}

	ks = 0;
	kd = ((y - 1)*dst_s + x - 1) * depth;
	_blit(src, dst, ks, kd, depth);

	ks = (src_w - 1) * depth;
	kd = ((y - 1)*dst_s + x + src_w) * depth;
	_blit(src, dst, ks, kd, depth);

	ks = (src_w * (src_h - 1)) * depth;
	kd = ((y + src_h)*dst_s + x - 1) * depth;
	_blit(src, dst, ks, kd, depth);

	ks = (src_w * src_h - 1) * depth;
	kd = ((y + src_h)*dst_s + x + src_w) * depth;
	_blit(src, dst, ks, kd, depth);

	for (int j = 0; j < src_w; ++j) {
		ks = j * depth;
		kd = ((y - 1)*dst_s + (x + j)) * depth;
		_blit(src, dst, ks, kd, depth);
		ks = ((src_h - 1)*src_w + j) * depth;
		kd = ((y + src_h)*dst_s + (x + j)) * depth;
		_blit(src, dst, ks, kd, depth);
	}

	for (int i = 0; i < src_h; ++i) {
		ks = (i*src_w) * depth;
		kd = ((y + i)*dst_s + (x - 1)) * depth;
		_blit(src, dst, ks, kd, depth);
		ks = (i*src_w + src_w - 1) * depth;
		kd = ((y + i)*dst_s + (x + src_w)) * depth;
		_blit(src, dst, ks, kd, depth);
	}

	for (int i = 0; i < src_h; ++i) {
		for (int j = 0; j < src_w; ++j) {
			ks = (i*src_w + j) * depth;
			kd = ((y + i)*dst_s + (x + j)) * depth;
			_blit(src, dst, ks, kd, depth);
		}
	}

	return 0;
}

static int
ltrimimg(lua_State *L) {
	int pixfmt = _check_pixel_format(luaL_checkstring(L, -1));
	int h = luaL_checkint(L, -2);
	int w = luaL_checkint(L, -3);
	uint8_t* src = lua_touserdata(L, -4);

	if (pixfmt != PIXEL_FORMAT_RGBA) {
		luaL_error(L, "cannot trim image that do not have alpha channel");
	}

	int l = 0;
	int r = 0;
	int t = 0;
	int b = 0;

	for (int x = 0; x < w; ++x) {
		int transparent = 1;
		for (int y = 0; y < h; ++y) {
			if (src[(y*w + x) * 4 + 3] > 0) {
				transparent = 0;
				break;
			}
		}
		if (!transparent) break;
		++l;
	}
	for (int x = w - 1; x >= 0; --x) {
		int transparent = 1;
		for (int y = 0; y < h; ++y) {
			if (src[(y*w + x) * 4 + 3] > 0) {
				transparent = 0;
				break;
			}
		}
		if (!transparent) break;
		++r;
	}
	for (int y = 0; y <= h; ++y) {
		int transparent = 1;
		for (int x = 0; x < w; ++x) {
			if (src[(y*w + x) * 4 + 3] > 0) {
				transparent = 0;
				break;
			}
		}
		if (!transparent) break;
		++t;
	}
	for (int y = h - 1; y >= 0; --y) {
		int transparent = 1;
		for (int x = 0; x < w; ++x) {
			if (src[(y*w + x) * 4 + 3] > 0) {
				transparent = 0;
				break;
			}
		}
		if (!transparent) break;
		++b;
	}

	if (l == 0 && r == 0 && t == 0 && b == 0) {
		return 0;
	}

	uint8_t* dst = lua_newuserdata(L, (w - l - r)*(h - t - b) * 4);
	for (int i = 0; i < h - t - b; ++i) {
		for (int j = 0; j < w - l - r; ++j) {
			int k1 = (i*(w - l - r) + j) * 4;
			int k2 = ((i + t)*w + j + l) * 4;
			dst[k1 + 0] = src[k2 + 0];
			dst[k1 + 1] = src[k2 + 1];
			dst[k1 + 2] = src[k2 + 2];
			dst[k1 + 3] = src[k2 + 3];
		}
	}

	lua_pushnumber(L, l);
	lua_pushnumber(L, r);
	lua_pushnumber(L, t);
	lua_pushnumber(L, b);

	return 5;
}

int
register_libimage(lua_State *L) {
	luaL_Reg l[] = {
		{ "loadpng", lloadpng },
		{ "savepng", lsavepng },
		{ "loadppm", lloadppm },
		{ "saveppm", lsaveppm },
		{ "newimg", lnewimg },
		{ "blitimg", lblitimg },
		{ "trimimg", ltrimimg },
		{ NULL, NULL }
	};

	luaL_newlib(L, l);
	return 1;
}