#include "image.h"

#include <stdlib.h>

#include "debugmalloc.h"

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

	return image;
}

void image_destroy(Image* image)
{
	free(image->pixels);
	free(image);
}

bool image_scale(Image* image, float horizontal, float vertical)
{
	uint32_t new_width = image->width * horizontal;
	uint32_t new_height = image->height * vertical;

	Pixel* pixels = (Pixel*)malloc(new_width * new_height * sizeof(Pixel));
	if (pixels == NULL)
	{
		return false;
	}

	for (uint32_t y = 0; y < new_height; y++)
	{
		for (uint32_t x = 0; x < new_width; x++)
		{
			//Pixel pixel;

			/* TODO */

			//pixels[y * new_width + x] = pixel;
		}
	}

	free(image->pixels);
	image->pixels = pixels;
	image->width = new_width;
	image->height = new_height;

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
	/* TODO */
	return true;
}

static uint8_t image_limit_pixel_component(int component)
{
	return (component < 0) ? 0 : (component > 255) ? 255 : component;
}

bool image_exposure(Image* image, int value)
{
	for (uint32_t y = 0; y < image->height; y++)
	{
		for (uint32_t x = 0; x < image->width; x++)
		{
			Pixel pixel = image->pixels[y * image->width + x];

			pixel.blue = image_limit_pixel_component(pixel.blue + value);
			pixel.green = image_limit_pixel_component(pixel.green + value);
			pixel.red = image_limit_pixel_component(pixel.red + value);

			image->pixels[y * image->width + x] = pixel;
		}
	}
	return true;
}
