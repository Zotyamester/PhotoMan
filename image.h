#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

typedef struct pixel_struct
{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
} Pixel;

typedef struct image_struct
{
	uint32_t width;
	uint32_t height;
	Pixel *pixels;
} Image;

Image* image_create(uint32_t width, uint32_t height);
void image_destroy(Image* image);

/* TODO: implementálni vagy hagyni a csudába */
Pixel* image_get_pixel(uint32_t row, uint32_t col);

int image_scale(Image* image, float horizontal, float vertical);
int image_mirror_x(Image* image);
int image_mirror_y(Image* image);
int image_blur(Image* image, int value);
int image_exposure(Image* image, int value);

#endif /* IMAGE_H_INCLUDED */
