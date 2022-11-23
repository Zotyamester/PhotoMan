/*****************************************************************//**
 * @file   status.c
 * @brief  A hibakezelő modul forrásfájlja.
 *
 * @author Zoltán Szatmáry
 * @date   November 2022
 *********************************************************************/
#include "status.h"
#include "image.h"
#include "bmp.h"
#include "cmd.h"

#include <stdio.h>

#include "debugmalloc.h"

/* az általánosan definiált hibakódok szöveges reprezentációja */
const char* status_error_code_strings[] = {
	"Nincs hiba.",
	"Nincs eleg memoria.",
	"I/O hiba."
};

/**
 * A támogatott modulok és a saját hibakódjait értelmezni és szövegesen
 * megjeleníteni képes függvény.
 * 
 * @param code A konzolban megjelenítendő hibakód.
 */
void status_print(int code)
{
	const char* error_string;

	if (code >= STATUS_ERROR_OFFSET && code < STATUS_ERROR_OFFSET + 1000)
		error_string = status_error_code_strings[code - STATUS_ERROR_OFFSET];
	else if (code >= IMAGE_ERROR_OFFSET && code < IMAGE_ERROR_OFFSET + 1000)
		error_string = image_error_code_strings[code - IMAGE_ERROR_OFFSET];
	else if (code >= BMP_ERROR_OFFSET && code < BMP_ERROR_OFFSET + 1000)
		error_string = bmp_error_code_strings[code - BMP_ERROR_OFFSET];
	else if (code >= CMD_ERROR_OFFSET && code < CMD_ERROR_OFFSET + 1000)
		error_string = cmd_error_code_strings[code - CMD_ERROR_OFFSET];
	else
		error_string = "Ismeretlen hibakod.";

	fprintf(stderr, "HIBA_%d: %s\n", code, error_string);
}
