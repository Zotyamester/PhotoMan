#include "bmp.h"

#include <stdlib.h>
#include <stdint.h>

struct file_header_struct
{
	uint16_t signature;
	uint32_t file_size;
	uint32_t reserved;
	uint32_t data_offset;
};

struct info_header_struct
{
	uint32_t header_size;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bits_per_pixel;
	uint32_t compression;
	uint32_t image_size;
	uint32_t x_pixels_per_m;
	uint32_t y_pixels_per_m;
	uint32_t colors_used;
	uint32_t important_colors;
};

struct color_entry
{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t reserved;
};

bool load_bmp(Image** p_image, FILE* file)
{
	struct file_header_struct fileheader;
	if (fread(&fileheader, sizeof(struct file_header_struct), 1, file) != 1)
		return false;

	// TODO: check validity

	struct info_header_struct infoheader;
	if (fread(&infoheader, sizeof(struct info_header_struct), 1, file) != 1)
		return false;

	struct color_entry* color_table = (struct color_entry*)malloc(infoheader.colors_used * sizeof(struct color_entry));
	if (color_table == NULL)
		return false;
	if (fread(color_table, sizeof(struct color_entry), infoheader.colors_used, file) != infoheader.colors_used)
	{
		free(color_table);
		return false;
	}

	if (fseek(file, fileheader.data_offset, SEEK_SET) != 0)
	{
		free(color_table);
		return false;
	}

	Image* image = image_create(infoheader.width, infoheader.height);
	if (image == NULL)
	{
		free(color_table);
		return false;
	}

	uint32_t row_width = (infoheader.width * infoheader.bits_per_pixel + 7) / 8;
	row_width += 4 - row_width % 4;
	uint8_t* row = (uint8_t*)malloc(row_width * sizeof(uint8_t));
	if (row == NULL)
	{
		free(image);
		free(color_table);
		return false;
	}

	uint32_t ptr = 0;
	while (fread(row, sizeof(uint8_t), row_width, file) == row_width)
	{
		uint64_t bitptr = 0;
		while (bitptr < 8 * row_width)
		{
			uint64_t bitendptr = bitptr + infoheader.bits_per_pixel;
			uint32_t pixeldata = 0; // TODO
			image->pixels[ptr++] = (infoheader.bits_per_pixel > 8) ? pixeldata : color_table[pixeldata];
			bitptr = bitendptr;
		}
	}

	free(row);
	free(color_table);
	*p_image = image;

	return true;
}

bool store_bmp(Image** p_image, FILE* file)
{
	// TODO
	return true;
}
