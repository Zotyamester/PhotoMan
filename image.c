/*****************************************************************//**
 * @file   image.c
 * @brief  Absztrakt képek kezelését megvalósító modul forrásfájlja.
 * 
 * @author Zoltán Szatmáry
 * @date   November 2022
 *********************************************************************/
#include "image.h"
#include "status.h"

#include <stdlib.h>

#include "debugmalloc.h"

/* az képek kezelésénél előjövő hibakódok szöveges reprezentációja */
const char* image_error_code_strings[] = {
	"Hibas parameter."
};

/**
 * Készít egy dinamikusan foglalt, width × height dimenziójú
 * pixelmátrixot - az annak indexelését segítő pointertömbbel együtt,
 * melyeket paraméterként ad vissza a hívónak, amennyiben sikeresek
 * voltak a foglalások.
 * 
 * A lefoglalt memóriaterület felszabadítása a hívó feladata.
 * 
 * @param[in] width A pixelmátrix szélessége.
 * @param[in] height A pixelmátrix magassága.
 * @param[out] p_pixel_data A pixelmátrixra mutató pointer helye.
 * @param[out] p_pixels A pixelmátrix pointertömbjére mutató pointer helye.
 * @return Sikeres lefutás esetén NO_ERROR, egyébként MEMORY_ERROR.
 */
static int image_create_pixel_matrix(uint32_t width, uint32_t height, Pixel** p_pixel_data, Pixel*** p_pixels)
{
	debugmalloc_max_block_size(width * height * sizeof(Pixel) + height * sizeof(Pixel*));

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

/**
 * Készít egy dinamikusan foglalt absztrakt képet tároló sturktúrát, mely
 * elkészítésekor egy width × height dimenziójú kép tárolására alkalmas.
 *  
 * A lefoglalt memóriaterület felszabadítása a hívó feladata.
 * 
 * @param width A kép szélessége.
 * @param height A kép magassága.
 * @return Sikeres lefutás esetén a dinamikusan foglalt stuktúrára mutató
 * pointer, foglalási hiba esetén pedig NULL-pointer.
 */
Image* image_create(uint32_t width, uint32_t height)
{
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

/**
 * Felszabadít egy dinamikusan foglalt absztrakt képet tároló struktúrát
 * annak minden dinamikusan foglalt memóriaterületével együtt.
 * 
 * @param image A felszabadítandó kép sturktúrára mutató pointer.
 */
void image_destroy(Image* image)
{
	free(image->pixels);
	free(image->pixel_data);
	free(image);
}

/**
 * Megadja, hogy egy dimenzió skálázása megvalósítható-e egyértelműen, vagyis
 * hogy a dimenziót a skálázási értékkel elosztva egész szám-e a hányados.
 * 
 * @param dimension A dimenzió.
 * @param scale A skálázási érték (skalár).
 * @return Amennyiben a dimenzió skálázása egyértelmű, logikai igazzal,
 * egyébként logikai hamissal tér vissza.
 */
static bool is_divisible(uint32_t dimension, float scale)
{
	uint32_t quotient = dimension / scale;
	uint32_t original = quotient * scale;
	return original == dimension;
}

/**
 * Fel- vagy leskáláz egy képet megadott függőleges és vízszintes
 * paraméterek szerint.
 * 
 * Az adott tengely szerint skálázás egynél nagyobb értékeknél a kép
 * nagyítását, egynél kisebb értékekre pedig a kép kicsinyítését idézi elő.
 * 
 * A függvény újrafoglalhat dinamikusan memóriaterületet, ilyenkor a korábbi
 * területeket felszabadítja, viszont az újonnan foglaltak felszabadítása
 * továbbra is a hívó feladata marad.
 * 
 * @param image A feldolgozandó kép.
 * @param horizontal A vízszintes skálázás értéke. Mindig pozitív.
 * @param vertical A függőleges skálázás értéke. Mindig pozitív.
 * @return Sikeres lefutás esetén NO_ERROR-ral, hibás paraméter esetén
 * IMAGE_BAD_PARAMETER-rel, memóriafoglalási hiba esetén pedig MEMORY_ERROR-ral
 * tér vissza.
 */
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

/**
 * Megcseréli két pixel értékét.
 * 
 * @param p_pixel1 Az egyik pixelre mutató pointer.
 * @param p_pixel2 A másik pixelre mutató pointer.
 */
static void swap_pixels(Pixel* p_pixel1, Pixel* p_pixel2)
{
	Pixel temp = *p_pixel1;
	*p_pixel1 = *p_pixel2;
	*p_pixel2 = temp;
}

/**
 * Tükröz egy képet az x tengelyre.
 * 
 * @param image A feldolgozandó kép.
 * @return Minden esetben NO_ERROR státusszal tér vissza.
 */
int image_mirror_x(Image* image)
{
	for (uint32_t x = 0; x < image->width; x++)
		for (uint32_t y = 0; y < image->height / 2; y++)
			swap_pixels(&image->pixels[y][x], &image->pixels[image->height - 1 - y][x]);
	return NO_ERROR;
}

/**
 * Tükröz egy képet az y tengelyre.
 *
 * @param image A feldolgozandó kép.
 * @return Minden esetben NO_ERROR státusszal tér vissza.
 */
int image_mirror_y(Image* image)
{
	for (uint32_t y = 0; y < image->height; y++)
		for (uint32_t x = 0; x < image->width / 2; x++)
			swap_pixels(&image->pixels[y][x], &image->pixels[y][image->width - 1 - x]);
	return NO_ERROR;
}

/**
 * Alkalmaz egy mátrix-szal megadott konvolúciót egy pixelmátrixra.
 * 
 * @param dst A cél pixelmátrix.
 * @param src A forrás pixelmátrix.
 * @param width A pixelmátrixok szélessége.
 * @param height A pixelmátrixok magassága.
 * @param coeff A konvolúciós mátrix együtthatója.
 * @param kernel A konvolúciós mátrix.
 */
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

/**
 * Elhomályosít vagy élesít egy képet megadott intenzitással.
 * 
 * @param image A feldolgozandó kép.
 * @param value Az művelet intenzitása. A művelet pozitív értékek esetén
 * elhomályosítás, negatív értékek esetén élesítés.
 * @return Sikeres lefutás esetén NO_ERROR-ral, hibás paraméterezés esetén
 * IMAGE_BAD_PARAMETER-rel, memóriafoglalási hiba esetén pedig
 * MEMORY_ERROR-ral tér vissza.
 */
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

/**
 * Limitálja egy 32 bites előjeles egészként megadott színkomponens értékét,
 * hogy az a 8 bites előjel nélküli számábrázolási tartományon belülre essen.
 * 
 * @param component A színkomponens.
 * @return Visszatér a tartományra limitált komponenssel.
 */
static uint8_t limit_pixel_component(int component)
{
	return (component < 0) ? 0 : (component > 255) ? 255 : component;
}

/**
 * Megnöveli, illetve lecsökkenti egy kép fényerejét megadott intenzitással.
 * 
 * @param image A feldolgozandó kép.
 * @param value Az művelet intenzitása.
 * @return Minden esetben NO_ERROR státusszal tér vissza.
 */
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
