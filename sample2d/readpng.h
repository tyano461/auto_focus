#ifndef __READPNG_H__
#define __READPNG_H__

#include <stdint.h>
#include <png.h>

int load_png(char *fname, uint32_t*** prgb, int32_t *width, int32_t *height);
int load_png_with_alpha(char *fname, uint32_t*** prgb, int32_t *width, int32_t *height);
void free_png(uint32_t **rgb);
#endif
