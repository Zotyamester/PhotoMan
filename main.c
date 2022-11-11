#include <stdio.h>
#include <string.h>
#include "status.h"
#include "image.h"
#include "bmp.h"

#include "debugmalloc.h"

/* TODO: ha minden kész, és maradt idõ: platformfüggetlenné tétel! */

int main(int argc, char* argv[])
{
	int status = NO_ERROR;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			printf("Options: ...\n");
			goto print_status;
		}
	}

	if (argc < 3)
	{
		/*status = TOO_FEW_ARGUMENTS;*/
		abort(1); // TODO
		goto print_status;
	}

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
		union {
			float scale;
			int value;
		} param;

		if (!(
			sscanf(argv[i], "-sx=%f", &param.scale) == 1 &&
			(status = image_scale(image, param.scale, 1.0)) == NO_ERROR ||
			sscanf(argv[i], "-sy=%f", &param.scale) == 1 &&
			(status = image_scale(image, 1.0, param.scale)) == NO_ERROR ||
			strcmp(argv[i], "-mx") == 0 &&
			(status = image_mirror_x(image)) == NO_ERROR ||
			strcmp(argv[i], "-my") == 0 &&
			(status = image_mirror_y(image)) == NO_ERROR ||
			sscanf(argv[i], "-b=%d", &param.value) == 1 &&
			(status = image_blur(image, param.value)) == NO_ERROR ||
			sscanf(argv[i], "-e=%d", &param.value) == 1 &&
			(status = image_exposure(image, param.value)) == NO_ERROR
			))
		{
			//status = CANNOT_EXECUTE_COMMAND;
			abort();
			goto destroy_image;
		}
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
