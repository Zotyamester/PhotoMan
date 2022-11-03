#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED

typedef enum status_enum
{
	OK = 0,
	SHOW_HELP,
	TOO_FEW_ARGUMENTS,
	UNABLE_TO_OPEN_FILE,
	CANNOT_LOAD_FILE,
	CANNOT_SAVE_FILE,
	CANNOT_EXECUTE_COMMAND
} Status;

void status_print(Status code);

#endif /* STATUS_H_INCLUDED */
