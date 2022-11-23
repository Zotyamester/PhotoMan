/*****************************************************************//**
 * @file   bmp.c
 * @brief  A BMP formátumú képek kezelését megvalósító modul
 * forrásfájlja.
 *
 * @author Zoltán Szatmáry
 * @date   November 2022
 *********************************************************************/
#include "bmp.h"
#include "image.h"
#include "status.h"

#include <stdlib.h>
#include <stdint.h>

#include "debugmalloc.h"

#define BMP_SIGNATURE			19778
#define BMP_PLANES_VALUE		1

#define BMP_FILE_HEADER_SIZE	14
#define BMP_INFO_HEADER_SIZE	40

 /* az BMP fájlok kezelésénél előjövő hibakódok szöveges reprezentációja */
const char* bmp_error_code_strings[] = {
	"Hibas fajlalairas.",
	"Nem tamogatott megjelenitesi beallitas.",
	"Hibas bitmelyseg."
};

/* BMP fájlfejlécet tároló struktúra */
struct file_header_struct
{
	uint16_t signature;

	uint16_t __padding; /* 4 bájtos padding (natural alignment) */

	uint32_t file_size;
	uint32_t reserved;
	uint32_t data_offset;
};

/* BMP információs fejlécet tároló struktúra */
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

/* BMP színtáblázat egy bejegyzését tároló stuktúra */
struct color_entry
{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t reserved;
};

/**
 * Megvizsgálja, hogy a fájlfejléc szabványos-e.
 * 
 * @param fileheader A fájlfejléc.
 * @return Amennyiben a szignatúra nem felel meg a szabványosnak,
 * BMP_INVALID_SIGNATURE-rel, egyébként pedig NO_ERROR-ral tér vissza.
 */
static int bmp_check_file_validity(struct file_header_struct* fileheader)
{
	if (fileheader->signature != BMP_SIGNATURE)
		return BMP_INVALID_SIGNATURE;
	return NO_ERROR;
}

/**
 * Megvizsgálja, hogy az információs fejléc szabványos-e.
 * 
 * @param infoheader Az információs fejléc.
 * @return Amennyiben sok fejléc "megjelenítés" mezője nem "képernyő"-re
 * van állítva, BMP_TOO_MANY_PLANES-zel, ha a színekkel kapcsolatos mezők
 * helytelenek, BMP_INVALID_COLORS-zal, egyébként pedig NO_ERROR-ral tér
 * vissza.
 */
static int bmp_check_info_validity(struct info_header_struct* infoheader)
{
	if (infoheader->planes != BMP_PLANES_VALUE)
		return BMP_TOO_MANY_PLANES;

	if (!(infoheader->important_colors <= infoheader->colors_used))
		return BMP_INVALID_COLORS;

	switch (infoheader->bits_per_pixel)
	{
	case 1:
		return (infoheader->colors_used == 1) ? NO_ERROR : BMP_INVALID_COLORS;
	case 4: case 8:
		return (infoheader->colors_used == (1u << infoheader->bits_per_pixel)) ? NO_ERROR : BMP_INVALID_COLORS;
	case 16: case 24:
		return NO_ERROR;
	default:
		return BMP_INVALID_COLORS;
	}
}

/**
 * Kiszámolja egy bittérképbeli sor hosszát 4 bájtos igazítással.
 * 
 * @param width A sor hossza (bájtban).
 * @param bits_per_pixel A pixelenkénti bitek száma, vagyis a bitmélység.
 * @return Visszatér a kiszámított sorhosszal.
 */
static uint32_t bmp_calculate_row_width(uint32_t width, uint16_t bits_per_pixel)
{
	/* (count * size + 4-byte-alignment) * 4 bytes */
	return ((width * bits_per_pixel + 31) / 32) * 4;
}

/* I/O műveletet megvalósító függvényekre mutató függvénypointer típus */
typedef size_t(*foperation)(void* buffer, size_t element_size, size_t element_count, FILE* stream);

/**
 * Elvégez egy I/O műveletet egy fájlfejléccel.
 * 
 * @param p_fileheader A fájlfejlécre mutató pointer.
 * @param file A fájl.
 * @param operation Az elvégzendő I/O művelet (fread/fwrite).
 * @return Sikeres lefutás esetén NO_ERROR-ral, műveletvégzés közben felmerülő
 * I/O probléma esetén IO_ERROR-ral tér vissza.
 */
static int bmp_rdwr_file_header(struct file_header_struct* p_fileheader, FILE* file, foperation operation)
{
	if (operation(&p_fileheader->signature, sizeof(p_fileheader->signature), 1, file) == 1 &&
		operation(&p_fileheader->file_size, sizeof(p_fileheader->file_size), 1, file) == 1 &&
		operation(&p_fileheader->reserved, sizeof(p_fileheader->reserved), 1, file) == 1 &&
		operation(&p_fileheader->data_offset, sizeof(p_fileheader->data_offset), 1, file) == 1)
		return NO_ERROR;
	return IO_ERROR;
}

/**
 * Beolvas egy fájlfejlécet egy fájlból.
 * 
 * @param p_fileheader A fájlfejlécre mutató pointer.
 * @param file A fájl.
 * @return Sikeres lefutás esetén NO_ERROR-ral, beolvasás közben felmerülő
 * I/O probléma esetén IO_ERROR-ral tér vissza.
 */
static int bmp_read_file_header(struct file_header_struct* p_fileheader, FILE* file)
{
	return bmp_rdwr_file_header(p_fileheader, file, fread);
}

/**
 * Kiír egy fájlfejlécet egy fájlba.
 *
 * @param p_fileheader A fájlfejlécre mutató pointer.
 * @param file A fájl.
 * @return Sikeres lefutás esetén NO_ERROR-ral, kiírás közben felmerülő
 * I/O probléma esetén IO_ERROR-ral tér vissza.
 */
static int bmp_write_file_header(struct file_header_struct* p_fileheader, FILE* file)
{
	return bmp_rdwr_file_header(p_fileheader, file, fwrite);
}

/**
 * Elvégez egy I/O műveletet egy információs fejléccel.
 *
 * @param p_infoheader Az információs fejlécre mutató pointer.
 * @param file A fájl.
 * @param operation Az elvégzendő I/O művelet (fread/fwrite).
 * @return Sikeres lefutás esetén NO_ERROR-ral, műveletvégzés közben felmerülő
 * I/O probléma esetén IO_ERROR-ral tér vissza.
 */
static int bmp_rdwr_info_header(struct info_header_struct* p_infoheader, FILE* file, foperation operation)
{
	if (operation(&p_infoheader->header_size, sizeof(p_infoheader->header_size), 1, file) == 1 &&
		operation(&p_infoheader->width, sizeof(p_infoheader->width), 1, file) == 1 &&
		operation(&p_infoheader->height, sizeof(p_infoheader->height), 1, file) == 1 &&
		operation(&p_infoheader->planes, sizeof(p_infoheader->planes), 1, file) == 1 &&
		operation(&p_infoheader->bits_per_pixel, sizeof(p_infoheader->bits_per_pixel), 1, file) == 1 &&
		operation(&p_infoheader->compression, sizeof(p_infoheader->compression), 1, file) == 1 &&
		operation(&p_infoheader->image_size, sizeof(p_infoheader->image_size), 1, file) == 1 &&
		operation(&p_infoheader->x_pixels_per_m, sizeof(p_infoheader->x_pixels_per_m), 1, file) == 1 &&
		operation(&p_infoheader->y_pixels_per_m, sizeof(p_infoheader->y_pixels_per_m), 1, file) == 1 &&
		operation(&p_infoheader->colors_used, sizeof(p_infoheader->colors_used), 1, file) == 1 &&
		operation(&p_infoheader->important_colors, sizeof(p_infoheader->important_colors), 1, file) == 1)
		return NO_ERROR;
	return IO_ERROR;
}

/**
 * Beolvas egy információs fejlécet egy fájlból.
 *
 * @param p_infoheader Az információs fejlécre mutató pointer.
 * @param file A fájl.
 * @return Sikeres lefutás esetén NO_ERROR-ral, beolvasás közben felmerülő
 * I/O probléma esetén IO_ERROR-ral tér vissza.
 */
static int bmp_read_info_header(struct info_header_struct* p_infoheader, FILE* file)
{
	return bmp_rdwr_info_header(p_infoheader, file, fread);
}

/**
 * Kiír egy információs fejlécet egy fájlba.
 *
 * @param p_infoheader Az információs fejlécre mutató pointer.
 * @param file A fájl.
 * @return Sikeres lefutás esetén NO_ERROR-ral, kiírás közben felmerülő
 * I/O probléma esetén IO_ERROR-ral tér vissza.
 */
static int bmp_write_info_header(struct info_header_struct* p_infoheader, FILE* file)
{
	return bmp_rdwr_info_header(p_infoheader, file, fwrite);
}

/**
 * Kivág egy bitszekvenciát egy 32 bites előjel nélküli egészeket tartalmazó
 * tömbből.
 * 
 * @param array A tömb.
 * @param start A legelső kivágandó bit indexe (0-tól számozva).
 * @param size A kivágandó bitszekvencia mérete (bitekben).
 * @return Visszatér a kivágott bitszekvenciát nullákkal kiegészítve kapott
 * 32 bites előjel nélküli egésszel.
 */
static uint32_t cut_bitseq_from_u32_array(const uint32_t* array, uint64_t start, uint16_t size)
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

/**
 * Betölt egy szabványos BMP formátumú képet egy fájlból, melyet paraméterként
 * ad vissza a hívónak.
 * 
 * A lefoglalt memóriaterület felszabadítása a hívó feladata.
 * 
 * @param p_image A képre mutató poitner helye.
 * @param file A fájl.
 * @return Sikeres lefutás esetén NO_ERROR-ral, egyébként a validálások,
 * az allokációk vagy egy I/O művelet által okozott hibakóddal tér vissza.
 */
int bmp_load(Image** p_image, FILE* file)
{
	int status;

	struct file_header_struct fileheader;

	if ((status = bmp_read_file_header(&fileheader, file)) != NO_ERROR)
		return status;
	if ((status = bmp_check_file_validity(&fileheader)) != NO_ERROR)
		return status;

	struct info_header_struct infoheader;
	if ((status = bmp_read_info_header(&infoheader, file)) != NO_ERROR)
		return status;
	if ((status = bmp_check_info_validity(&infoheader)) != NO_ERROR)
		return status;

	struct color_entry* color_table = NULL;
	if (infoheader.colors_used > 0)
	{
		color_table = (struct color_entry*)malloc(infoheader.colors_used * sizeof(struct color_entry));
		if (color_table == NULL)
			return MEMORY_ERROR;

		if (fread(color_table, sizeof(struct color_entry), infoheader.colors_used, file) != infoheader.colors_used)
		{
			free(color_table);
			return IO_ERROR;
		}
	}

	if (fseek(file, fileheader.data_offset, SEEK_SET) != 0)
	{
		free(color_table);
		return IO_ERROR;
	}

	Image* image = image_create(infoheader.width, infoheader.height);
	if (image == NULL)
	{
		free(color_table);
		return MEMORY_ERROR;
	}

	uint32_t row_width = bmp_calculate_row_width(infoheader.width, infoheader.bits_per_pixel);
	uint32_t* row = (uint32_t*)malloc(row_width * sizeof(uint8_t));
	if (row == NULL)
	{
		free(color_table);
		image_destroy(image);
		return MEMORY_ERROR;
	}

	uint32_t idx = 0;
	while (fread(row, sizeof(uint8_t), row_width, file) == row_width)
	{
		for (uint64_t bitptr = 0; bitptr < infoheader.width * infoheader.bits_per_pixel; bitptr += infoheader.bits_per_pixel)
		{
			Pixel pixel;

			uint32_t pixeldata = cut_bitseq_from_u32_array(row, bitptr, infoheader.bits_per_pixel);
			if (infoheader.bits_per_pixel == 1)
			{
				/* egy monokróm képnél egyetlen egy színt tárolunk a színtáblázatban,
				szóval vagy azt a színt reprezentálja a bit vagy a feketét */
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

			image->pixel_data[idx++] = pixel;
		}
	}

	*p_image = image;

	free(row);
	free(color_table);

	return NO_ERROR;
}

/**
 * Kiment egy szabványos BMP formátumú képet egy fájlba, melyet paraméterként
 * vesz át.
 *
 * @param p_image A képre mutató poitner helye.
 * @param file A fájl.
 * @return Sikeres lefutás esetén NO_ERROR-ral, egyébként az allokációk vagy
 * egy I/O művelet által okozott hibakóddal tér vissza.
 */
int bmp_store(const Image** p_image, FILE* file)
{
	int status;

	const Image* image = *p_image;

	struct info_header_struct infoheader = {
		.header_size = BMP_INFO_HEADER_SIZE,
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
		.file_size = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE + infoheader.image_size,
		.reserved = 0,
		.data_offset = BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE
	};

	if ((status = bmp_write_file_header(&fileheader, file)) != NO_ERROR)
		return status;
	if ((status = bmp_write_info_header(&infoheader, file)) != NO_ERROR)
		return status;

	uint32_t padding_size = row_width - infoheader.width * 3;
	uint8_t* padding = NULL;
	if (padding_size > 0)
	{
		padding = (uint8_t*)calloc(padding_size, sizeof(uint8_t));
		if (padding == NULL)
			return MEMORY_ERROR;
	}

	for (Pixel** p_row = image->pixels; p_row < image->pixels + infoheader.height; p_row++)
	{
		if (fwrite(*p_row, 3, infoheader.width, file) != infoheader.width ||
			fwrite(padding, sizeof(uint8_t), padding_size, file) != padding_size)
		{
			free(padding);
			return IO_ERROR;
		}
	}

	free(padding);

	return NO_ERROR;
}
