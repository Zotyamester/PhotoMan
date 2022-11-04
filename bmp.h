#ifndef BMP_H_INCLUDED
#define BMP_H_INCLUDED

#include <stdio.h>
#include <stdbool.h>
#include "image.h"

bool bmp_load(Image** p_image, FILE* file);
bool bmp_store(Image** p_image, FILE* file);

#endif /* BMP_H_INCLUDED */
