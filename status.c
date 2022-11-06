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
		fputs("Cannot load file", stderr);
		break;
	case CANNOT_SAVE_FILE:
		fputs("Cannot save file", stderr);
		break;
	case CANNOT_EXECUTE_COMMAND:
		fputs("Cannot execute command", stderr);
		break;
	default:
		printf("Not implemented yet. (%s : %d)\n", __FILE__, __LINE__);
		break;
	}
}
