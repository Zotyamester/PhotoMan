/*****************************************************************//**
 * @file   status.c
 * @brief  A hibakezel� modul forr�sf�jlja.
 *
 * @author Zolt�n Szatm�ry
 * @date   November 2022
 *********************************************************************/
#include "status.h"
#include "image.h"
#include "bmp.h"
#include "cmd.h"

#include <stdio.h>

#include "debugmalloc.h"

/* az �ltal�nosan defini�lt hibak�dok sz�veges reprezent�ci�ja */
const char* status_error_code_strings[] = {
	"Nincs hiba.",
	"Nincs eleg memoria.",
	"I/O hiba."
};

/**
 * A t�mogatott modulok �s a saj�t hibak�djait �rtelmezni �s sz�vegesen
 * megjelen�teni k�pes f�ggv�ny.
 * 
 * @param code A konzolban megjelen�tend� hibak�d.
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
