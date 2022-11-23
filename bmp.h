/*****************************************************************//**
 * @file   bmp.h
 * @brief  A BMP form�tum� k�pek kezel�s�t megval�s�t� modul
 * fejl�cf�jlja.
 * 
 * @author Zolt�n Szatm�ry
 * @date   November 2022
 *********************************************************************/
#ifndef BMP_H_INCLUDED
#define BMP_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>
#include "image.h"

#define BMP_ERROR_OFFSET	    2000

#define BMP_INVALID_SIGNATURE	2000
#define BMP_TOO_MANY_PLANES		2001
#define BMP_INVALID_COLORS		2002

extern const char* bmp_error_code_strings[];

int bmp_load(Image** p_image, FILE* file);
int bmp_store(Image** p_image, FILE* file);

#endif /* BMP_H_INCLUDED */
