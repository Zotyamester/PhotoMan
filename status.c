#include "status.h"

#include <stdio.h>

void status_print(Status code)
{
	switch (code)
	{
	default:
		printf("Not implemented yet. (%s : %d)\n", __FILE__, __LINE__);
		break;
	}
}
