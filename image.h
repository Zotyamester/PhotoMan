/*****************************************************************//**
 * @file   image.h
 * @brief  Absztrakt k�pek kezel�s�t megval�s�t� modul fejl�cf�jlja.
 * 
 * @author Zolt�n Szatm�ry
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
 * @brief Egy legfeljebb 24 bites, v�r�s, z�ld �s k�k RGB sz�nkomponensekb�l�ll�
 * sz�nt le�r� strukt�ra.
 */
typedef struct pixel_struct
{
	uint8_t blue; /* k�k */
	uint8_t green; /* z�ld */
	uint8_t red; /* v�r�s */
} Pixel;

/**
 * @brief Egy absztrakt k�pstrukt�ra, mely egy � a 32 bites el�jel n�lk�li
 * eg�sz sz�m�br�zol�si korl�tjait�l eltekintve � tetsz�legesen nagy
 * dimenzi�j�, legfeljebb 8 bites RGB komponensekkel rendelkez� k�p �s jellemz�
 * param�tereinek (sz�less�g, magass�g) egys�gbez�r�s�ra alkalmas.
 * 
 * A k�ppontokat a c�maritmetik�t helyettes�t� pixels sorc�m-lek�pzett
 * pointert�mb�n kereszt�l is el lehet �rni.
 */
typedef struct image_struct
{
	uint32_t width; /* k�psz�less�g */
	uint32_t height; /* k�pmagass�g */
	Pixel* pixel_data; /* pixelm�trix */
	Pixel** pixels; /* pointert�mb a pixelm�trix soraira */
} Image;

/* a k�pstukt�ra kezel�s�t megval�s�t� f�ggv�nyek */

Image* image_create(uint32_t width, uint32_t height);
void image_destroy(Image* image);

/* az elemi k�pmanipul�ci�kat megval�s�t� f�ggv�nyek */

int image_scale(Image* image, float horizontal, float vertical);
int image_mirror_x(Image* image);
int image_mirror_y(Image* image);
int image_blur(Image* image, int value);
int image_exposure(Image* image, int value);

#endif /* IMAGE_H_INCLUDED */
