/*****************************************************************//**
 * @file   cmd.c
 * @brief  Parancssori argumentumok kezel�s�t �s valid�l�s�t v�gz�
 * f�ggv�nyeket megval�s�t� modul forr�sf�jlja.
 *
 * @author Zolt�n Szatm�ry
 * @date   November 2022
 *********************************************************************/
#include "cmd.h"
#include "status.h"

#include <string.h>

/* az argumentumok kezel�s�n�l el�j�v� hibak�dok sz�veges reprezent�ci�ja */
const char* cmd_error_code_strings[] = {
	"Tul keves argumentum.",
	"Ismeretlen parancssori kapcsolo."
};

/**
 * Megvizsg�lja, hogy a parancssorb�l �rkez� argumentumok
 * sz�moss�ga megfelel-e az elv�rtnak.
 *
 * @param argc A parancssori argumentumok sz�ma.
 * @param desired Az elv�rt sz�moss�g.
 * @return Az ellen�rz�s eredm�ny�t t�kr�z� st�tusszal
 * t�r vissza, vagyis elt�r�s eset�n CMD_TOO_FEW_ARGUMENTS
 * hibak�ddal, egy�bk�nt pedig NO_ERROR-ral.
 */
int cmd_check_argc(int argc, int desired)
{
	return (argc < desired) ? CMD_TOO_FEW_ARGUMENTS : NO_ERROR;
}

/**
 * Megvizsg�lja, hogy megtal�lhat�-e az argumentum�rt�ket reprezent�l� sztring
 * az argumentumvektorban.
 * 
 * @param argv Az argumentumvektor.
 * @param arg A keresett sztring.
 * @return Amennyiben megtal�lhat� a sztring a vektorban, logikai igazzal,
 * egy�bk�nt logikai hamissal t�r vissza.
 */
bool cmd_find_argument(const char* argv[], const char* arg)
{
	while (*argv != NULL && strcmp(*argv, arg) != 0)
		argv++;
	return *argv != NULL;
}

/**
 * �rtelmezi a sztringk�nt megadott kapcsol�t, �s amennyiben lehets�ges,
 * v�grehajtja az ahhoz t�rs�tott m�veletet a megadott k�pen.
 * 
 * @param image A feldolgozand� k�p.
 * @param sw A m�velet parancssori kapcsol�j�t tartalmaz� sztring.
 * @return Sikeres lefut�s eset�n NO_ERROR-ral, egy�bk�nt a m�veletekhez
 * tartoz� st�tuszjellel/hibak�ddal, vagy CMD_UNKNOWN_CMD_SWITCH-csel
 * t�r vissza, amennyiben nem ismeret vagy hib�s a megadott kapcsol�.
 */
int cmd_parse_manip_switch(Image* image, const char* sw)
{
	int status = NO_ERROR;

	/* lok�lis uni� t�pus a param�teres kapcsol�k param�tereinek t�rol�s�ra */
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
