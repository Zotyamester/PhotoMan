#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED

#define STATUS_ERROR_OFFSET 0000

#define NO_ERROR		    0000
#define MEMORY_ERROR	    0001
#define IO_ERROR		    0002

extern const char* status_error_code_strings[];

void status_print(int code);

#endif /* STATUS_H_INCLUDED */
