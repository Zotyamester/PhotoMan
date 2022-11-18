#include "image.h"
#include "status.h"

#include <stdlib.h>

#include "debugmalloc.h"

const char* image_error_code_strings[] = {
	"Hibas parameter."
};

static int image_create_pixel_matrix(uint32_t width, uint32_t height, Pixel** p_pixel_data, Pixel*** p_pixels)
{
	Pixel* pixel_data = (Pixel*)malloc(width * height * sizeof(Pixel));
	if (pixel_data == NULL)
		return MEMORY_ERROR;

	Pixel** pixels = (Pixel**)malloc(height * sizeof(Pixel*));
	if (pixels == NULL)
	{
		free(pixel_data);
		return MEMORY_ERROR;
	}

	for (uint32_t i = 0; i < height; i++)
		pixels[i] = &pixel_data[i * width + 0];

	*p_pixel_data = pixel_data;
	*p_pixels = pixels;

	return NO_ERROR;
}

Image* image_create(uint32_t width, uint32_t height)
{
	debugmalloc_max_block_size(sizeof(Image) + width * height * sizeof(Pixel) + height * sizeof(Pixel*));

	Image* image = (Image*)malloc(sizeof(Image));
	if (image == NULL)
		return NULL;

	if (image_create_pixel_matrix(width, height, &image->pixel_data, &image->pixels) != NO_ERROR)
	{
		free(image);
		return NULL;
	}

	image->width = width;
	image->height = height;

	return image;
}

void image_destroy(Image* image)
{
	free(image->pixels);
	free(image->pixel_data);
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
	int status;

	if (!(is_divisible(image->width, horizontal) && is_divisible(image->height, vertical)))
		return IMAGE_BAD_PARAMETER;

	uint32_t new_width = image->width * horizontal;
	uint32_t new_height = image->height * vertical;

	Pixel* pixel_data;
	Pixel** pixels;

	if ((status = image_create_pixel_matrix(new_width, new_height, &pixel_data, &pixels)) != NO_ERROR)
		return status;

	for (uint32_t i = 0; i < new_height; i++)
		pixels[i] = &pixel_data[i * new_width + 0];

	for (uint32_t y_new = 0; y_new < new_height; y_new++)
	{
		uint32_t y_old = y_new / vertical;
		for (uint32_t x_new = 0; x_new < new_width; x_new++)
		{
			uint32_t x_old = x_new / horizontal;
			pixels[y_new][x_new] = image->pixels[y_old][x_old];
		}
	}

	free(image->pixels);
	free(image->pixel_data);

	image->pixel_data = pixel_data;
	image->pixels = pixels;
	image->width = new_width;
	image->height = new_height;

	return NO_ERROR;
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
			swap_pixels(&image->pixels[y][x], &image->pixels[image->height - 1 - y][x]);
	return NO_ERROR;
}

int image_mirror_y(Image* image)
{
	for (uint32_t y = 0; y < image->height; y++)
		for (uint32_t x = 0; x < image->width / 2; x++)
			swap_pixels(&image->pixels[y][x], &image->pixels[y][image->width - 1 - x]);
	return NO_ERROR;
}

static void pixel_apply_kernel(Pixel** dst, Pixel** src, uint32_t width, uint32_t height, float coeff, const int kernel[3][3])
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
					blue += kernel[i][j] * src[y + i][x + j].blue;
					green += kernel[i][j] * src[y + i][x + j].green;
					red += kernel[i][j] * src[y + i][x + j].red;
				}
			}

			Pixel pixel = { .blue = blue * coeff, .green = green * coeff, .red = red * coeff };

			dst[y + 1][x + 1] = pixel;
		}
	}

	for (uint32_t y = 0; y < height; y++)
	{
		dst[y][0] = src[y][0];
		dst[y][width - 1] = src[y][width - 1];
	}

	for (uint32_t x = 0; x < width; x++)
	{
		dst[0][x] = src[0][x];
		dst[height - 1][x] = src[height - 1][x];
	}
}

/* TODO: reménytelenül lassú nagy value-kra, sharpening nem működik */
int image_blur(Image* image, int value)
{
	int status;

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
		return IMAGE_BAD_PARAMETER;

	Pixel* src_data = image->pixel_data;
	Pixel** src = image->pixels;

	Pixel* dst_data;
	Pixel** dst;

	if ((status = image_create_pixel_matrix(image->width, image->height, &dst_data, &dst)) != NO_ERROR)
		return status;

	for (uint32_t i = 0; i < image->height; i++)
		dst[i] = &dst_data[i * image->width + 0];

	float coeff = 1.0 / 16.0;
	int(*kernel)[3] = blur;
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

		Pixel* tmp_data = dst_data;
		dst_data = src_data;
		src_data = tmp_data;

		Pixel** tmp = dst;
		dst = src;
		src = tmp;
	}

	image->pixel_data = dst_data;
	image->pixels = dst;

	free(src);
	free(src_data);

	return NO_ERROR;
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
			Pixel pixel = image->pixels[y][x];

			pixel.blue = limit_pixel_component(pixel.blue + value);
			pixel.green = limit_pixel_component(pixel.green + value);
			pixel.red = limit_pixel_component(pixel.red + value);

			image->pixels[y][x] = pixel;
		}
	}

	return NO_ERROR;
}
