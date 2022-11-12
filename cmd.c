#include "cmd.h"
#include "status.h"

#include <string.h>

const char* cmd_error_code_strings[] = {
	"Tul keves argumentum.",
	"Ismeretlen parancssori kapcsolo."
};

int cmd_check_argc(int argc, int desired)
{
	return (argc < desired) ? CMD_TOO_FEW_ARGUMENTS : NO_ERROR;
}

bool cmd_find_argument(const char* argv[], const char* arg)
{
	while (*argv != NULL && strcmp(*argv, arg) != 0)
		argv++;
	return *argv != NULL;
}

int cmd_parse_manip_switch(Image* image, const char* sw)
{
	int status = NO_ERROR;

	union {
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
