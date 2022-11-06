#include "image.h"

#include <stdlib.h>

#include "debugmalloc.h"

Image* image_create(uint32_t width, uint32_t height)
{
	debugmalloc_max_block_size(7680 * 4320 * sizeof(Pixel));

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

static bool is_divisible(uint32_t dimension, float scale)
{
	uint32_t quotient = dimension / scale;
	uint32_t original = quotient * scale;
	return original == dimension;
}

bool image_scale(Image* image, float horizontal, float vertical)
{
	if (!(is_divisible(image->width, horizontal) && is_divisible(image->height, vertical)))
		return false;

	uint32_t new_width = image->width * horizontal;
	uint32_t new_height = image->height * vertical;

	Pixel* pixels = (Pixel*)malloc(new_width * new_height * sizeof(Pixel));
	if (pixels == NULL)
	{
		return false;
	}

	for (uint32_t y_new = 0; y_new < new_height; y_new++)
	{
		uint32_t y_old = y_new / vertical;
		for (uint32_t x_new = 0; x_new < new_width; x_new++)
		{
			uint32_t x_old = x_new / horizontal;
			pixels[y_new * new_width + x_new] = image->pixels[y_old * image->width + x_old];
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
