#include "status.h"

#include <stdio.h>

void status_print(Status code)
{
	switch (code)
	{
	case OK:
		break;
	case SHOW_HELP:
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
		break;
	default:
		printf("Not implemented yet. (%s : %d)\n", __FILE__, __LINE__);
		break;
	}
}
