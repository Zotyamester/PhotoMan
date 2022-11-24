/*****************************************************************//**
 * @file   cmd.c
 * @brief  Parancssori argumentumok kezelését és validálását végző
 * függvényeket megvalósító modul forrásfájlja.
 *
 * @author Zoltán Szatmáry
 * @date   November 2022
 *********************************************************************/
#include "cmd.h"
#include "status.h"

#include <stdio.h>
#include <string.h>

/* az argumentumok kezelésénél előjövő hibakódok szöveges reprezentációja */
const char* cmd_error_code_strings[] = {
	"Tul keves argumentum.",
	"Ismeretlen parancssori kapcsolo."
};

/**
 * Megvizsgálja, hogy a parancssorból érkező argumentumok
 * számossága megfelel-e az elvártnak.
 *
 * @param argc A parancssori argumentumok száma.
 * @param desired Az elvárt számosság.
 * @return Az ellenőrzés eredményét tükröző státusszal
 * tér vissza, vagyis eltérés esetén CMD_TOO_FEW_ARGUMENTS
 * hibakóddal, egyébként pedig NO_ERROR-ral.
 */
int cmd_check_argc(int argc, int desired)
{
	return (argc < desired) ? CMD_TOO_FEW_ARGUMENTS : NO_ERROR;
}

/**
 * Megvizsgálja, hogy megtalálható-e az argumentumértéket reprezentáló sztring
 * az argumentumvektorban.
 * 
 * @param argv Az argumentumvektor.
 * @param arg A keresett sztring.
 * @return Amennyiben megtalálható a sztring a vektorban, logikai igazzal,
 * egyébként logikai hamissal tér vissza.
 */
bool cmd_find_argument(const char* argv[], const char* arg)
{
	while (*argv != NULL && strcmp(*argv, arg) != 0)
		argv++;
	return *argv != NULL;
}

/**
 * Értelmezi a sztringként megadott kapcsolót, és amennyiben lehetséges,
 * végrehajtja az ahhoz társított műveletet a megadott képen.
 * 
 * @param image A feldolgozandó kép.
 * @param sw A művelet parancssori kapcsolóját tartalmazó sztring.
 * @return Sikeres lefutás esetén NO_ERROR-ral, egyébként a műveletekhez
 * tartozó státuszjellel/hibakóddal, vagy CMD_UNKNOWN_CMD_SWITCH-csel
 * tér vissza, amennyiben nem ismeret vagy hibás a megadott kapcsoló.
 */
int cmd_parse_manip_switch(Image* image, const char* sw)
{
	int status = NO_ERROR;

	/* lokális unió típus a paraméteres kapcsolók paramétereinek tárolására */
	union switch_paramter {
		float scale;
		int value;
	} param;

	if (sscanf(sw, "-sx=%f", &param.scale) == 1)
		status = image_scale(image, param.scale, 1.0);
	else if (sscanf(sw, "-sy=%f", &param.scale) == 1)
		status = image_scale(image, 1.0, param.scale);
	else if (strcmp(sw, "-mx") == 0)
		status = image_mirror_x(image);
	else if (strcmp(sw, "-my") == 0)
		status = image_mirror_y(image);
	else if (sscanf(sw, "-b=%d", &param.value) == 1)
		status = image_blur(image, param.value);
	else if (sscanf(sw, "-e=%d", &param.value) == 1)
		status = image_exposure(image, param.value);
	else
		status = CMD_UNKNOWN_CMD_SWITCH;

	return status;
}
