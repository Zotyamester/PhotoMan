#include "image.h"

#include <stdlib.h>

Image* image_create(uint32_t width, uint32_t height)
{
	Image* image = (Image*)malloc(sizeof(Image));
	if (image == NULL)
		return NULL;

	Pixel* pixels = (Pixel*)malloc(width * height * sizeof(Pixel));
	if (pixels == NULL)
	{
		free(image);
		return NULL;
	}

	image->pixels = pixels;
	image->width = width;
	image->height = height;
	image->format_specific_data = NULL;
}

void image_destroy(Image* image)
{
	free(image->format_specific_data);
	free(image->pixels);
	free(image);
}

bool image_scale(Image* image, float horizontal, float vertical)
{
	// TODO
	return true;
}

void swap_pixels(Pixel* p_pixel1, Pixel* p_pixel2)
{
	Pixel temp = *p_pixel1;
	*p_pixel1 = *p_pixel2;
	*p_pixel2 = temp;
}

bool image_mirror_x(Image* image)
{
	for (uint32_t x = 0; x < image->width; x++)
		for (uint32_t y = 0; y < image->height / 2; y++)
			swap_pixels(&image->pixels[y * image->width + x], &image->pixels[(image->height - 1 - y) * image->width + x]);
	return true;
}

bool image_mirror_y(Image* image)
{
	for (uint32_t y = 0; y < image->height; y++)
		for (uint32_t x = 0; x < image->width / 2; x++)
			swap_pixels(&image->pixels[y * image->width + x], &image->pixels[y * image->width + image->width - 1 - x]);
	return true;
}

bool image_blur(Image* image, int value)
{
	// TODO
	return true;
}

bool image_exposure(Image* image, int value)
{
	// TODO
	return true;
}
