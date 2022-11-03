#ifndef BMP_H_INCLUDED
#define BMP_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>
#include "image.h"

bool load_bmp(Image** p_image, FILE* file);
bool store_bmp(Image** p_image, FILE* file);

#endif /* BMP_H_INCLUDED */
