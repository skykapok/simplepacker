#ifndef __IMG_PPM_H__
#define __IMG_PPM_H__

#define PPM_UNKNOWN 0
#define PPM_RGBA8 1
#define PPM_RGB8 2
#define PPM_RGBA4 3
#define PPM_RGB4 4
#define PPM_ALPHA8 5
#define PPM_ALPHA4 6

struct ppm {
	int type;
	int depth;
	int step;
	int width;
	int height;
	uint8_t *buffer;
};

int img_loadppm(const char* filename, struct ppm* ppm);
int img_saveppm(const char* filename, int format, int width, int height, uint8_t* buffer);

#endif
