#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED

/* TODO: státuszkezelést át kéne variálni:
 * - int-ekkel térjenek vissza a függvények
 * - a modulok külön hibakódokkal és hibakódot szöveggé alakító függvénnyel (vagy kódokat szöveggé mappelő tömbbel?) rendelkezzenekű
 * - status_enum ne legyen
 */

#define STATUS_ERROR_OFFSET 0000

#define NO_ERROR		    0000
#define MEMORY_ERROR	    0001
#define IO_ERROR		    0002

extern char* status_error_code_strings[];

void status_print(int code);

#endif /* STATUS_H_INCLUDED */
