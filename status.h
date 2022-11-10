#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED

/* TODO: st�tuszkezel�st �t k�ne vari�lni:
 * - int-ekkel t�rjenek vissza a f�ggv�nyek
 * - a modulok k�l�n hibak�dokkal �s hibak�dot sz�vegg� alak�t� f�ggv�nnyel (vagy k�dokat sz�vegg� mappel� t�mbbel?) rendelkezzenek�
 * - status_enum ne legyen
 */

#define NOERR 10000

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
