#include "bmp.h"
#include "image.h"

#include <stdlib.h>
#include <stdint.h>

#include "debugmalloc.h"

#define BMP_SIGNATURE 19778

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

/* TODO: értelmesebb validálás, esetleg külön modulba áthelyezve?*/
static bool bmp_check_file_validity(struct file_header_struct* fileheader)
{
	return fileheader->signature == BMP_SIGNATURE;
}

static bool bmp_check_info_validity(struct info_header_struct* infoheader)
{
	if (!(infoheader->planes == 1))
		return false;

	if (!(infoheader->colors_used == 0 && infoheader->important_colors == 0 ||
		infoheader->important_colors <= infoheader->colors_used))
		return false;

	switch (infoheader->bits_per_pixel)
	{
	case 1:
		return infoheader->colors_used == 1;
	case 4: case 8:
		return infoheader->colors_used == 1 << infoheader->bits_per_pixel;
	case 16: case 24:
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

static bool bmp_read_file_header(struct file_header_struct* p_fileheader, FILE* file)
{
	return (
		fread(&p_fileheader->signature, sizeof(p_fileheader->signature), 1, file) == 1 &&
		fread(&p_fileheader->file_size, sizeof(p_fileheader->file_size), 1, file) == 1 &&
		fread(&p_fileheader->reserved, sizeof(p_fileheader->reserved), 1, file) == 1 &&
		fread(&p_fileheader->data_offset, sizeof(p_fileheader->data_offset), 1, file) == 1
		);
}

static bool bmp_write_file_header(struct file_header_struct* p_fileheader, FILE* file)
{
	return (
		fwrite(&p_fileheader->signature, sizeof(p_fileheader->signature), 1, file) == 1 &&
		fwrite(&p_fileheader->file_size, sizeof(p_fileheader->file_size), 1, file) == 1 &&
		fwrite(&p_fileheader->reserved, sizeof(p_fileheader->reserved), 1, file) == 1 &&
		fwrite(&p_fileheader->data_offset, sizeof(p_fileheader->data_offset), 1, file) == 1
		);
}

static bool bmp_read_info_header(struct info_header_struct* p_infoheader, FILE* file)
{
	return (
		fread(&p_infoheader->header_size, sizeof(p_infoheader->header_size), 1, file) == 1 &&
		fread(&p_infoheader->width, sizeof(p_infoheader->width), 1, file) == 1 &&
		fread(&p_infoheader->height, sizeof(p_infoheader->height), 1, file) == 1 &&
		fread(&p_infoheader->planes, sizeof(p_infoheader->planes), 1, file) == 1 &&
		fread(&p_infoheader->bits_per_pixel, sizeof(p_infoheader->bits_per_pixel), 1, file) == 1 &&
		fread(&p_infoheader->compression, sizeof(p_infoheader->compression), 1, file) == 1 &&
		fread(&p_infoheader->image_size, sizeof(p_infoheader->image_size), 1, file) == 1 &&
		fread(&p_infoheader->x_pixels_per_m, sizeof(p_infoheader->x_pixels_per_m), 1, file) == 1 &&
		fread(&p_infoheader->y_pixels_per_m, sizeof(p_infoheader->y_pixels_per_m), 1, file) == 1 &&
		fread(&p_infoheader->colors_used, sizeof(p_infoheader->colors_used), 1, file) == 1 &&
		fread(&p_infoheader->important_colors, sizeof(p_infoheader->important_colors), 1, file) == 1
		);
}

static bool bmp_write_info_header(struct info_header_struct* p_infoheader, FILE* file)
{
	return (
		fwrite(&p_infoheader->header_size, sizeof(p_infoheader->header_size), 1, file) == 1 &&
		fwrite(&p_infoheader->width, sizeof(p_infoheader->width), 1, file) == 1 &&
		fwrite(&p_infoheader->height, sizeof(p_infoheader->height), 1, file) == 1 &&
		fwrite(&p_infoheader->planes, sizeof(p_infoheader->planes), 1, file) == 1 &&
		fwrite(&p_infoheader->bits_per_pixel, sizeof(p_infoheader->bits_per_pixel), 1, file) == 1 &&
		fwrite(&p_infoheader->compression, sizeof(p_infoheader->compression), 1, file) == 1 &&
		fwrite(&p_infoheader->image_size, sizeof(p_infoheader->image_size), 1, file) == 1 &&
		fwrite(&p_infoheader->x_pixels_per_m, sizeof(p_infoheader->x_pixels_per_m), 1, file) == 1 &&
		fwrite(&p_infoheader->y_pixels_per_m, sizeof(p_infoheader->y_pixels_per_m), 1, file) == 1 &&
		fwrite(&p_infoheader->colors_used, sizeof(p_infoheader->colors_used), 1, file) == 1 &&
		fwrite(&p_infoheader->important_colors, sizeof(p_infoheader->important_colors), 1, file) == 1
		);
}

static uint32_t cut_bitseq_from_u32_array(const uint32_t *array, uint64_t start, uint16_t size)
{
	const uint32_t from = start;
	const uint32_t to = start + size;

	const uint32_t from_idx = from / 32;
	const uint32_t from_offset = from % 32;
	const uint32_t from_size = (size < (32 - from % 32)) ? size : (32 - from % 32);

	uint32_t bitseq = 0;
	
	const uint32_t lowerbits = (array[from_idx] >> from_offset) & ((1 << from_size) - 1);

	bitseq |= lowerbits;

	if (from_size < size)
	{
		const uint32_t to_idx = to / 32;
		const uint32_t to_offset = 0;
		const uint32_t to_size = to % 32;

		const uint32_t higherbits = (array[to_idx] >> to_offset) & ((1 << to_size) - 1);

		bitseq |= higherbits << from_size;
	}

	return bitseq;
}

int bmp_load(Image** p_image, FILE* file)
{
	struct file_header_struct fileheader;
	if (!bmp_read_file_header(&fileheader, file) ||
		!bmp_check_file_validity(&fileheader))
		return false;

	struct info_header_struct infoheader;
	if (!bmp_read_info_header(&infoheader, file) ||
		!bmp_check_info_validity(&infoheader))
		return false;

	struct color_entry* color_table = NULL;
	if (infoheader.colors_used > 0)
	{
		color_table = (struct color_entry*)malloc(infoheader.colors_used * sizeof(struct color_entry));
		if (color_table == NULL)
			return false;
	}

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
		
		for (uint64_t bitptr = 0; bitptr < infoheader.width * infoheader.bits_per_pixel; bitptr += infoheader.bits_per_pixel)
		{
			Pixel pixel;

			uint32_t pixeldata = cut_bitseq_from_u32_array(row, bitptr, infoheader.bits_per_pixel);
			if (infoheader.bits_per_pixel == 1)
			{
				/* a monochrome image has exactly one color in the table, so we either display that or nothing (black) */
				struct color_entry* color = &color_table[0];
				pixel.blue = pixeldata * color->blue;
				pixel.green = pixeldata * color->green;
				pixel.red = pixeldata * color->red;
			}
			else if (infoheader.bits_per_pixel <= 8)
			{
				struct color_entry* color = &color_table[pixeldata];
				pixel.blue = color->blue;
				pixel.green = color->green;
				pixel.red = color->red;
			}
			else
			{
				pixel.blue = (pixeldata) & 0xFF;
				pixel.green = (pixeldata >>= 8) & 0xFF;
				pixel.red = (pixeldata >>= 8);
			}

			image->pixels[ptr++] = pixel;
		}
	}

	free(row);
	free(color_table);
	*p_image = image;

	return true;
}

int bmp_store(Image** p_image, FILE* file)
{
	Image* image = *p_image;

	struct info_header_struct infoheader = {
		.header_size = INFO_HEADER_SIZE,
		.width = image->width,
		.height = image->height,
		.planes = 1,
		.bits_per_pixel = 24, /* only 24 bit outputs are supported */
		.compression = 0, /* only uncompressed outputs are supported */
		/* .image_size = ..., */
		.x_pixels_per_m = 0, /* printed size is unsupported */
		.y_pixels_per_m = 0, /* printed size is unsupported */
		.colors_used = 0, /* color table is unsupported*/
		.important_colors = 0 /* color table is unsupported */
	};

	uint32_t row_width = bmp_calculate_row_width(infoheader.width, infoheader.bits_per_pixel);
	infoheader.image_size = infoheader.height * row_width;

	struct file_header_struct fileheader = {
		.signature = BMP_SIGNATURE,
		.file_size = FILE_HEADER_SIZE + INFO_HEADER_SIZE + infoheader.image_size,
		.reserved = 0,
		.data_offset = FILE_HEADER_SIZE + INFO_HEADER_SIZE
	};

	if (!bmp_write_file_header(&fileheader, file))
		return false;

	if (!bmp_write_info_header(&infoheader, file))
		return false;

	uint32_t padding_size = row_width - infoheader.width * 3;
	uint8_t* padding = NULL;
	if (padding_size > 0)
	{
		padding = (uint8_t*)calloc(padding_size, sizeof(uint8_t));
		if (padding == NULL)
			return false;
	}

	for (Pixel* p_row = image->pixels; p_row < image->pixels + infoheader.height * infoheader.width; p_row += infoheader.width)
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
