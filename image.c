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

int image_scale(Image* image, float horizontal, float vertical)
{
	if (!(is_divisible(image->width, horizontal) && is_divisible(image->height, vertical)))
		return false;

	uint32_t new_width = image->width * horizontal;
	uint32_t new_height = image->height * vertical;

	Pixel* pixels = (Pixel*)malloc(new_width * new_height * sizeof(Pixel));
	if (pixels == NULL)
		return false;

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

static void swap_pixels(Pixel* p_pixel1, Pixel* p_pixel2)
{
	Pixel temp = *p_pixel1;
	*p_pixel1 = *p_pixel2;
	*p_pixel2 = temp;
}

int image_mirror_x(Image* image)
{
	for (uint32_t x = 0; x < image->width; x++)
		for (uint32_t y = 0; y < image->height / 2; y++)
			swap_pixels(&image->pixels[y * image->width + x], &image->pixels[(image->height - 1 - y) * image->width + x]);
	return true;
}

int image_mirror_y(Image* image)
{
	for (uint32_t y = 0; y < image->height; y++)
		for (uint32_t x = 0; x < image->width / 2; x++)
			swap_pixels(&image->pixels[y * image->width + x], &image->pixels[y * image->width + image->width - 1 - x]);
	return true;
}

static void pixel_apply_kernel(Pixel* dst, Pixel* src, uint32_t width, uint32_t height, float coeff, const int kernel[3][3])
{
	for (uint32_t y = 0; y < height - 2; y++)
	{
		for (uint32_t x = 0; x < width - 2; x++)
		{
			int blue = 0, green = 0, red = 0;

			for (uint32_t i = 0; i < 3; i++)
			{
				for (uint32_t j = 0; j < 3; j++)
				{
					blue += kernel[i][j] * src[(y + i) * width + (x + j)].blue;
					green += kernel[i][j] * src[(y + i) * width + (x + j)].green;
					red += kernel[i][j] * src[(y + i) * width + (x + j)].red;
				}
			}

			Pixel pixel = { .blue = blue * coeff, .green = green * coeff, .red = red * coeff };

			dst[(y + 1) * width + (x + 1)] = pixel;
		}
	}

	for (uint32_t y = 0; y < height; y++)
	{
		dst[y * width + 0] = src[y * width + 0];
		dst[y * width + width - 1] = src[y * width + width - 1];
	}

	for (uint32_t x = 0; x < width; x++)
	{
		dst[0 * width + x] = src[0 * width + x];
		dst[(height - 1) * width + x] = src[(height - 1) * width + x];
	}
}

/* TODO: reménytelenül lassú nagy value-kra, sharpening nem mûködik */
int image_blur(Image* image, int value)
{
	static const int blur[3][3] = {
		1, 2, 1,
		2, 4, 2,
		1, 2, 1
	};
	static const int sharpen[3][3] = {
		0, -1, 0,
		-1, 5, -1,
		0, -1, 0
	};

	if (value == 0)
		return true;

	Pixel* src = image->pixels;
	Pixel* dst = (Pixel*)malloc(image->width * image->height * sizeof(Pixel));
	if (dst == NULL)
		return false;

	float coeff = 1.0 / 16.0;
	int (*kernel)[3] = blur;
	if (value < 0)
	{
		coeff = 1.0;
		kernel = sharpen;
		value *= -1;
	}

	for (;;)
	{
		pixel_apply_kernel(dst, src, image->width, image->height, coeff, kernel);
		value -= 1;
		if (value == 0)
			break;
		Pixel* tmp = dst;
		dst = src;
		src = tmp;
	}

	image->pixels = dst;

	free(src);

	return true;
}

static uint8_t limit_pixel_component(int component)
{
	return (component < 0) ? 0 : (component > 255) ? 255 : component;
}

int image_exposure(Image* image, int value)
{
	for (uint32_t y = 0; y < image->height; y++)
	{
		for (uint32_t x = 0; x < image->width; x++)
		{
			Pixel pixel = image->pixels[y * image->width + x];

			pixel.blue = limit_pixel_component(pixel.blue + value);
			pixel.green = limit_pixel_component(pixel.green + value);
			pixel.red = limit_pixel_component(pixel.red + value);

			image->pixels[y * image->width + x] = pixel;
		}
	}
	return true;
}
