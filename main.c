/*****************************************************************//**
 * @file   main.c
 * @brief  A f�program.
 * 
 * @author Zolt�n Szatm�ry
 * @date   November 2022
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include "status.h"
#include "image.h"
#include "bmp.h"
#include "cmd.h"

#include "debugmalloc.h"

/**
 * A program bel�p�si pontja.
 * Itt t�rt�nik
 *   - az argumentumok valid�l�sa (r�szben),
 *   - az er�forr�sok kezel�se (allok�torok, deallok�torok vez�rl�se),
 *   - az argumentumokban meghat�rozott m�veletek megh�v�sa.
 * @param argc Argumentumok sz�ma bele�rtve a futtathat� bin�ris nev�t.
 * @param argv A NULL-termin�lt arugmentumvektor.
 * @return A program visszat�r�si k�dja, mely sikeres lefut�s eset�n
 * nulla, egy�bk�nt egy k�ls� (az oper�ci�s rendszer �ltal gener�lt)
 * vagy bels� (a program �ltal gener�lt) hibak�d.
 */
int main(int argc, char* argv[])
{
	int status = NO_ERROR;

	if (cmd_find_argument(argv + 1, "-h"))
	{
		const char* help_string = "Hasznalat: photoman <kep_be> <kep_ki> [opciok]\n"
			"Alapveto manipulaciot kepes vegezni egy BMP formatumu kepen.\n\n"
			"Bemeneti vagy kimeneti fajl hijan csak a sugot kepes kiirni.\n\n"
			"Opciok:\n"
			"  -h: kiirja a program rovid hasznalati utmutatojat, benne foglalva az osszes kapcsolot\n"
			"  -s<xy>=parameter: horizontalis skalazas\n"
			"  -m<xy>: tukrozes az x/y tengelyre\n"
			"  -b=parameter: Gauss-elmosas merteke\n"
			"  -e=parameter: expozicio eltolasanak merteke (negativ - sotetit, pozitiv - vilagosit)";
		puts(help_string);
		goto print_status;
	}

	if ((status = cmd_check_argc(argc, 3)) != NO_ERROR)
		goto print_status;

	FILE* input_file = fopen(argv[1], "rb");
	if (input_file == NULL)
	{
		status = IO_ERROR;
		goto print_status;
	}

	FILE* output_file = fopen(argv[2], "wb");
	if (output_file == NULL)
	{
		status = IO_ERROR;
		goto close_input;
	}

	Image* image;

	if ((status = bmp_load(&image, input_file)) != NO_ERROR)
		goto close_output;

	for (int i = 3; i < argc; i++)
	{
		status = cmd_parse_manip_switch(image, argv[i]);
		if (status != NO_ERROR)
			goto destroy_image;
	}

	if ((status = bmp_store(&image, output_file)) != NO_ERROR)
		goto destroy_image;

destroy_image:
	image_destroy(image);
close_output:
	fflush(output_file);
	fclose(output_file);
close_input:
	fclose(input_file);
print_status:
	if (status != NO_ERROR)
		status_print(status);

	return status;
}
