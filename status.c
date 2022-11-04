#include "status.h"

#include <stdio.h>

#include "debugmalloc.h"

void status_print(Status code)
{
	switch (code)
	{
	case OK:
		break;
	case SHOW_HELP:
		puts("Usage: photoman <img_in> <img_out> [options]");
		break;
	case TOO_FEW_ARGUMENTS:
		fputs("Too few arguments.", stderr);
		break;
	case UNABLE_TO_OPEN_FILE:
		perror("Cannot open file");
		break;
	case CANNOT_LOAD_FILE:
		perror("Cannot load file");
		break;
	case CANNOT_SAVE_FILE:
		perror("Cannot save file");
		break;
	case CANNOT_EXECUTE_COMMAND:
		perror("Cannot execute command");
		break;
	default:
		printf("Not implemented yet. (%s : %d)\n", __FILE__, __LINE__);
		break;
	}
}
