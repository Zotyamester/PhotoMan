#ifndef CMD_H_INCLUDED
#define CMD_H_INCLUDED

#include <stdbool.h>
#include "image.h"

#define CMD_ERROR_OFFSET		3000

#define CMD_TOO_FEW_ARGUMENTS	3000
#define CMD_UNKNOWN_CMD_SWITCH	3001

extern const char* cmd_error_code_strings[];

int cmd_check_argc(int argc, int desired);
bool cmd_find_argument(const char* argv[], const char* arg);
int cmd_parse_manip_switch(Image* image, const char* sw);

#endif /* CMD_H_INCLUDED */
