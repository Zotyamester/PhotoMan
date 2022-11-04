#include "bmp.h"
#include "image.h"

#include <stdlib.h>
#include <stdint.h>

#define FILE_HEADER_SIZE 14
#define INFO_HEADER_SIZE 40

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

static bool bmp_check_file_validity(struct file_header_struct* fileheader)
{
	return fileheader->signature == 19778;
}

static bool bmp_check_info_validity(struct info_header_struct* infoheader)
{
	if (!(infoheader->header_size == 40 && infoheader->planes == 1))
		return false;

	if (!(infoheader->colors_used == 0 && infoheader->important_colors == 0 ||
		infoheader->important_colors <= infoheader->colors_used))
		return false;

	switch (infoheader->bits_per_pixel)
	{
	case 1:
		return infoheader->colors_used == 1;
	case 4: case 8: case 16:
		return infoheader->colors_used == 1 << infoheader->bits_per_pixel;
	case 24:
		return true;
	default:
		return false;
	}
}

static uint32_t bmp_calculate_row_width(uint32_t width, uint16_t bits_per_pixel)
{
	/* (count * size + 4-byte-alignment) * 4 bytes */
	return ((width * bits_per_pixel + 31) / 32) * 4;
}

bool bmp_load(Image** p_image, FILE* file)
{
	struct file_header_struct fileheader;
	if (fread(&fileheader, FILE_HEADER_SIZE, 1, file) != 1)
		return false;

	if (!bmp_check_file_validity(&fileheader))
		return false;

	struct info_header_struct infoheader;
	if (fread(&infoheader, INFO_HEADER_SIZE, 1, file) != 1)
		return false;

	if (!bmp_check_info_validity(&infoheader))
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

	uint32_t row_width = bmp_calculate_row_width(infoheader.width, infoheader.bits_per_pixel);
	uint32_t* row = (uint32_t*)malloc(row_width * sizeof(uint8_t));
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
		while (bitptr < infoheader.width * infoheader.bits_per_pixel)
		{
			uint64_t bitendptr = bitptr + infoheader.bits_per_pixel;

			uint32_t lower = (row[bitptr / 32] >> (bitptr % 32)) & ((1 << infoheader.bits_per_pixel) - 1);
			uint32_t higher = row[bitendptr / 32] & ((1 << (32 - bitptr % 32)) - 1);
			uint32_t pixeldata = lower | higher << (bitptr % 32);

			Pixel pixel;
			if (infoheader.bits_per_pixel <= 8)
			{
				pixel.blue = color_table[pixeldata].blue;
				pixel.green = color_table[pixeldata].green;
				pixel.red = color_table[pixeldata].red;
			}
			else
			{
				pixel.blue = (pixeldata) & 0xFF;
				pixel.green = (pixeldata >>= 8) & 0xFF;
				pixel.red = (pixeldata >>= 8);
			}
			image->pixels[ptr++] = pixel;

			bitptr = bitendptr;
		}
	}

	free(row);
	free(color_table);
	*p_image = image;

	return true;
}

bool bmp_store(Image** p_image, FILE* file)
{
	Image* image = *p_image;

	struct info_header_struct infoheader = {
		.header_size = 40,
		.width = image->width,
		.height = image->height,
		.planes = 1,
		.bits_per_pixel = 24, /* only 24 bit outputs are supported */
		.compression = 0, /* only uncompressed outputs are supported */
		.image_size = 0, /* while there's no support for compression, it's zero */
		.x_pixels_per_m = 0, /* printed size is unsupported */
		.y_pixels_per_m = 0, /* printed size is unsupported */
		.colors_used = 0, /* color table is unsupported*/
		.important_colors = 0 /* color table is unsupported */
	};

	uint32_t row_width = bmp_calculate_row_width(infoheader.width, infoheader.bits_per_pixel);

	struct file_header_struct fileheader = {
		.signature = 19778,
		.file_size = FILE_HEADER_SIZE + INFO_HEADER_SIZE + infoheader.height * row_width,
		.reserved = 0,
		.data_offset = FILE_HEADER_SIZE + INFO_HEADER_SIZE
	};

	if (fwrite(&fileheader, FILE_HEADER_SIZE, 1, file) != 1)
		return false;

	if (fwrite(&infoheader, INFO_HEADER_SIZE, 1, file) != 1)
		return false;

	uint32_t padding_size = row_width - infoheader.width * 3;
	uint8_t* padding = (uint8_t)calloc(padding_size, sizeof(uint8_t));
	if (padding == NULL)
		return false;

	for (Pixel *p_row = image->pixels; p_row < image->pixels + infoheader.height * infoheader.width; p_row += infoheader.width)
	{
		if (fwrite(p_row, 3, infoheader.width, file) != infoheader.width ||
			fwrite(padding, sizeof(uint8_t), padding_size, file) != padding_size)
		{
			free(padding);
			return false;
		}
	}

	free(padding);

	return true;
}
