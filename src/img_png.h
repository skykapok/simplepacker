#ifndef __IMG_PNG_H__
#define __IMG_PNG_H__

#define PNG_UNKNOWN 0
#define PNG_RGBA 1
#define PNG_RGB 2

struct png {
	int type;
	int channel;
	int width;
	int height;
	uint8_t *buffer;
};

int img_loadpng(const char* filename, struct png* png);
int img_savepng(const char* filename, int format, int width, int height, uint8_t* buffer);

#endif
