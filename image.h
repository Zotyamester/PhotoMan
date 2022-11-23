/*****************************************************************//**
 * @file   image.h
 * @brief  Absztrakt képek kezelését megvalósító modul fejlécfájlja.
 * 
 * @author Zoltán Szatmáry
 * @date   November 2022
 *********************************************************************/
#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#define IMAGE_ERROR_OFFSET			1000

#define IMAGE_BAD_PARAMETER			1000

extern const char* image_error_code_strings[];

/**
 * @brief Egy legfeljebb 24 bites, vörös, zöld és kék RGB színkomponensekbőlálló
 * színt leíró struktúra.
 */
typedef struct pixel_struct
{
	uint8_t blue; /* kék */
	uint8_t green; /* zöld */
	uint8_t red; /* vörös */
} Pixel;

/**
 * @brief Egy absztrakt képstruktúra, mely egy – a 32 bites előjel nélküli
 * egész számábrázolási korlátjaitól eltekintve – tetszőlegesen nagy
 * dimenziójú, legfeljebb 8 bites RGB komponensekkel rendelkező kép és jellemző
 * paramétereinek (szélesség, magasság) egységbezárására alkalmas.
 * 
 * A képpontokat a címaritmetikát helyettesítő pixels sorcím-leképzett
 * pointertömbön keresztül is el lehet érni.
 */
typedef struct image_struct
{
	uint32_t width; /* képszélesség */
	uint32_t height; /* képmagasság */
	Pixel* pixel_data; /* pixelmátrix */
	Pixel** pixels; /* pointertömb a pixelmátrix soraira */
} Image;

/* a képstuktúra kezelését megvalósító függvények */

Image* image_create(uint32_t width, uint32_t height);
void image_destroy(Image* image);

/* az elemi képmanipulációkat megvalósító függvények */

int image_scale(Image* image, float horizontal, float vertical);
int image_mirror_x(Image* image);
int image_mirror_y(Image* image);
int image_blur(Image* image, int value);
int image_exposure(Image* image, int value);

#endif /* IMAGE_H_INCLUDED */
