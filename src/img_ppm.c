#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "img_ppm.h"

#define LINEMAX 128

static char*
readline(FILE* f, char* buffer) {
	for (;;) {
		char* ret = fgets(buffer, LINEMAX, f);
		if (ret == NULL) {
			return NULL;
		}
		if (ret[0] != '#') {
			return ret;
		}
	}
}

static int
ppm_header(FILE* f, struct ppm* ppm) {
	char tmp[LINEMAX];

	char* line = readline(f, tmp);
	if (line == NULL)
		return 0;
	char c = 0;
	sscanf(line, "P%c", &c);
	ppm->type = c;

	line = readline(f, tmp);
	if (line == NULL)
		return 0;
	sscanf(line, "%d %d", &(ppm->width), &(ppm->height));

	line = readline(f, tmp);
	if (line == NULL)
		return 0;
	sscanf(line, "%d", &(ppm->depth));

	return 1;
}

static int
ppm_data(struct ppm* ppm, FILE* f, int id, int skip) {
	int i;
	int n = ppm->width * ppm->height;
	uint8_t* buffer = ppm->buffer + skip;
	uint8_t* tmp;
	int step = ppm->step;
	switch(id) {
	case '3':	// RGB text
		for (i=0;i<n;i++) {
			int r,g,b;
			fscanf(f, "%d %d %d", &r,&g,&b);
			buffer[i*step+0] = (uint8_t)r;
			buffer[i*step+1] = (uint8_t)g;
			buffer[i*step+2] = (uint8_t)b;
		}
		break;
	case '2':	// ALPHA text
		for (i=0;i<n;i++) {
			int alpha;
			fscanf(f, "%d", &alpha);
			buffer[i*step] = (uint8_t)alpha;
		}
		break;
	case '6':	// RGB binary
		tmp = (uint8_t*)malloc(n * 3);
		if (fread(tmp, n*3, 1, f)==0) {
			free(tmp);
			return 0;
		}
		for (i=0;i<n;i++) {
			buffer[i*step+0] = tmp[i*3+0];
			buffer[i*step+1] = tmp[i*3+1];
			buffer[i*step+2] = tmp[i*3+2];
		}
		free(tmp);
		break;
	case '5':	// ALPHA binary
		tmp = (uint8_t*)malloc(n);
		if (fread(tmp, n, 1, f)==0) {
			free(tmp);
			return 0;
		}
		for (i=0;i<n;i++) {
			buffer[i*step] = tmp[i];
		}
		free(tmp);
		break;
	default:
		return 0;
	}
	return 1;
}

static int
loadppm_from_file(FILE* rgb, FILE* alpha, struct ppm* ppm) {
	ppm->buffer = NULL;
	ppm->step = 0;
	int rgb_id = 0;
	int alpha_id = 0;
	if (rgb) {
		if (!ppm_header(rgb, ppm)) {
			return 0;
		}
		rgb_id = ppm->type;
		ppm->step += 3;
	}
	if (alpha) {
		if (rgb == NULL) {
			if (!ppm_header(alpha, ppm)) {
				return 0;
			}
			alpha_id = ppm->type;
		} else {
			struct ppm pgm;
			if (!ppm_header(alpha, &pgm)) {
				return 0;
			}
			if (ppm->depth != pgm.depth || ppm->width != pgm.width || ppm->height != pgm.height) {
				return 0;
			}
			alpha_id = pgm.type;
		}
		ppm->step += 1;
	}
	ppm->buffer = (uint8_t*)malloc(ppm->height * ppm->width * ppm->step);
	if (rgb) {
		if (!ppm_data(ppm, rgb, rgb_id, 0)) {
			return 0;
		}
	}
	if (alpha) {
		int skip = 0;
		if (rgb) {
			skip = 3;
		}
		if (!ppm_data(ppm, alpha, alpha_id, skip)) {
			return 0;
		}
	}

	return 1;
}

int
img_loadppm(const char* filename, struct ppm* ppm) {
	char* tmp = malloc(sizeof(char)* (strlen(filename) + 5));
	sprintf(tmp, "%s.ppm", filename);
	FILE *rgb = fopen(tmp, "rb");
	sprintf(tmp, "%s.pgm", filename);
	FILE *alpha = fopen(tmp, "rb");
	free(tmp);

	if (rgb == NULL && alpha == NULL) {
		return 0;
	}

	int ok = loadppm_from_file(rgb, alpha, ppm);

	if (rgb) {
		fclose(rgb);
	}
	if (alpha) {
		fclose(alpha);
	}

	if (!ok) {
		if (ppm->buffer) {
			free(ppm->buffer);
		}
		return 0;
	}

	if (ppm->depth == 255) {
		if (ppm->step == 4) {
			ppm->type = PPM_RGBA8;
		}
		else if (ppm->step == 3) {
			ppm->type = PPM_RGB8;
		}
		else {
			ppm->type = PPM_ALPHA8;
		}
	}
	else {
		if (ppm->step == 4) {
			ppm->type = PPM_RGBA4;
		}
		else if (ppm->step == 3) {
			ppm->type = PPM_RGB4;
		}
		else {
			ppm->type = PPM_ALPHA4;
		}
	}

	return 1;
}

static void
ppm_type(int format, struct ppm* ppm) {
	switch (format) {
	case PPM_RGBA8:
		ppm->type = PPM_RGBA8;
		ppm->depth = 255;
		ppm->step = 4;
		break;

	case PPM_RGB8:
		ppm->type = PPM_RGB8;
		ppm->depth = 255;
		ppm->step = 3;
		break;

	case PPM_RGBA4:
		ppm->type = PPM_RGBA4;
		ppm->depth = 15;
		ppm->step = 4;
		break;

	case PPM_RGB4:
		ppm->type = PPM_RGB4;
		ppm->depth = 15;
		ppm->step = 3;
		break;

	case PPM_ALPHA8:
		ppm->type = PPM_ALPHA8;
		ppm->depth = 255;
		ppm->step = 1;
		break;

	case PPM_ALPHA4:
		ppm->type = PPM_ALPHA4;
		ppm->depth = 15;
		ppm->step = 1;
		break;

	default:
		ppm->type = PPM_UNKNOWN;
		ppm->depth = 0;
		ppm->step = 0;
		break;
	}
}

static int
save_rgb(const char* filename, struct ppm ppm) {
	char* tmp = malloc(sizeof(char)* (strlen(filename) + 5));
	sprintf(tmp, "%s.ppm", filename);
	FILE *f = fopen(tmp,"wb");
	free(tmp);
	if (f == NULL) {
		return 0;
	}

	fprintf(f,
		"P6\n"
		"%d %d\n"
		"%d\n"
		, ppm.width, ppm.height, ppm.depth);

	uint8_t* buffer = (uint8_t*)malloc(ppm.width * ppm.height * 3);
	for (int i=0; i<ppm.width*ppm.height; ++i) {
		buffer[i * 3 + 0] = ppm.buffer[i*ppm.step + 0];
		buffer[i * 3 + 1] = ppm.buffer[i*ppm.step + 1];
		buffer[i * 3 + 2] = ppm.buffer[i*ppm.step + 2];
	}

	int wn = fwrite(buffer, 3, ppm.width*ppm.height, f);
	free(buffer);
	fclose(f);
	if (wn != 1) {
		return 0;
	}

	return 1;
}

static int
save_alpha(const char* filename, struct ppm ppm, int offset) {
	char* tmp = malloc(sizeof(char)* (strlen(filename) + 5));
	sprintf(tmp, "%s.pgm", filename);
	FILE *f = fopen(tmp, "wb");
	free(tmp);
	if (f == NULL) {
		return 0;
	}

	fprintf(f,
		"P5\n"
		"%d %d\n"
		"%d\n"
		, ppm.width, ppm.height, ppm.depth);

	uint8_t *buffer = (uint8_t*)malloc(ppm.width * ppm.height);
	for (int i=0; i<ppm.width*ppm.height; ++i) {
		buffer[i] = ppm.buffer[i*ppm.step + offset];
	}

	int wn = fwrite(buffer, 1, ppm.width*ppm.height, f);
	free(buffer);
	fclose(f);
	if (wn != 1) {
		return 0;
	}

	return 1;
}

int
img_saveppm(const char* filename, int format, int width, int height, uint8_t* buffer) {
	struct ppm ppm;
	ppm_type(format, &ppm);
	ppm.width = width;
	ppm.height = height;
	ppm.buffer = buffer;
	if (ppm.type != PPM_ALPHA8 && ppm.type != PPM_ALPHA4) {
		save_rgb(filename, ppm);
	}
	if (ppm.type != PPM_RGB8 && ppm.type != PPM_RGB4) {
		int offset = 3;
		if (ppm.type ==  PPM_ALPHA8 || ppm.type == PPM_ALPHA4) {
			offset = 0;
		}
		save_alpha(filename, ppm, offset);
	}

	return 1;
}
