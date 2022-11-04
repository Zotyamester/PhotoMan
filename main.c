#include <stdio.h>
#include <string.h>
#include "status.h"
#include "image.h"
#include "bmp.h"

#include "debugmalloc.h"

int main(int argc, char* argv[])
{
	Status status = OK;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			status = SHOW_HELP;
			goto print_status;
		}
	}

	if (argc < 3)
	{
		status = TOO_FEW_ARGUMENTS;
		goto print_status;
	}

	FILE* input_file = fopen(argv[1], "rb");
	if (input_file == NULL)
	{
		status = UNABLE_TO_OPEN_FILE;
		goto print_status;
	}

	FILE* output_file = fopen(argv[2], "wb");
	if (output_file == NULL) {
		status = UNABLE_TO_OPEN_FILE;
		goto close_input;
	}

	Image* image;

	if (!bmp_load(&image, input_file))
	{
		status = CANNOT_LOAD_FILE;
		goto close_output;
	}

	for (int i = 3; i < argc; i++)
	{
		union {
			float scale;
			int value;
		} param;

		if (!(
			sscanf(argv[i], "-sx=%f", &param.scale) == 1 && image_scale(image, param.scale, 1.0) ||
			sscanf(argv[i], "-sy=%f", &param.scale) == 1 && image_scale(image, 1.0, param.scale) ||
			strcmp(argv[i], "-mx") == 0 && image_mirror_x(image) ||
			strcmp(argv[i], "-my") == 0 && image_mirror_y(image) ||
			sscanf(argv[i], "-b=%d", &param.value) == 1 && image_blur(image, param.value) ||
			sscanf(argv[i], "-e=%d", &param.value) == 1 && image_exposure(image, param.value)
			))
		{
			status = CANNOT_EXECUTE_COMMAND;
			goto destroy_image;
		}
	}

	if (!bmp_store(&image, output_file))
	{
		status = CANNOT_SAVE_FILE;
		goto destroy_image;
	}

destroy_image:
	image_destroy(image);
close_output:
	fflush(output_file);
	fclose(output_file);
close_input:
	fclose(input_file);
print_status:
	status_print(status);

	return status;
}
