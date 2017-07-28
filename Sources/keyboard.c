#include <mc9s08sh8.h>
#include "keyevent.h"
#include "keyboard.h"

// NOT_PRESSED es un valor de control utilizado para las funciones internas de keyboard
// que indica que la columna o fila no esta presionada
#define NOT_PRESSED 42 //meaning of life

// EMPTY_KEY es lo correspondiente a un valor nulo
const char EMPTY_KEY = ' ';

// variables internas utilizadas para la identificacion de la tecla presionada
char last_pressed_key;
char pressed_row;
char pressed_column;

// mapeo de teclas correspondiente al teclado matricial
const char keymap[4][4] = { { '1', '2', '3', 'A' },
                         { '4', '5', '6', 'B' },
                         { '7',	'8', '9', 'C' },
                         { '*', '0', '#', 'D' } };

void check_column(void); 
void check_row(void);

// keyboard_init inicializa las configuraciones del puerto B
// en el que esta conectado el teclado matricial
void keyboard_init(void){
	PTBDD = 0x0F;
	PTBPE = 0xF0;
	last_pressed_key=EMPTY_KEY;
}

// keyboard_check_key se encarga de implementar el algoritmo de 
// identificacion de la tecla presionada. Si se detecta el presionado
// y liberado de una tecla se pushea el valor de la tecla al buffer
// keyevent.
void keyboard_check_key(void) {
	check_column();
	if (pressed_column == NOT_PRESSED) {
		if (last_pressed_key == EMPTY_KEY)
			return;
		keyevent_push(last_pressed_key);
		last_pressed_key = EMPTY_KEY;
		return;
	}
	check_row();
	if (pressed_row == NOT_PRESSED) {
		return;
	}
	last_pressed_key = keymap[pressed_row][pressed_column];
}

// check_row de forma analoga, devuelve en pressed_row la fila correspondiente a la 
// tecla presionada. Realiza la deteccion, realizando un "barrido" de filas.
void check_row(void) {
	char current_row = 0;
	while (current_row < 4) {
		PTBD=0x0F;
		switch(current_row) {
		case 0:
			PTBD_PTBD0=0;
			break;
		case 1:
			PTBD_PTBD1=0;
			break;
		case 2:
			PTBD_PTBD2=0;
			break;
		case 3:
			PTBD_PTBD3=0;
			break;
		}
		check_column();
		if(pressed_column!=NOT_PRESSED) {
			pressed_row=current_row;
			PTBD=0x00;
			return;
		}
		current_row++;
	}
	pressed_row = NOT_PRESSED;
}

// check_column realiza el primer chequeo para la identificacion de la tecla presionada.
// Devuelve en pressed_column la columna correspondiente a la tecla presionada, si es
// que lo hubiese
void check_column(void) {
	if (PTBD_PTBD7 == 0) {
		pressed_column = 3;
		return;
	}
	if (PTBD_PTBD6 == 0) {
		pressed_column = 2;
		return;
	}
	if (PTBD_PTBD5 == 0) {
		pressed_column = 1;
		return;
	}
	if (PTBD_PTBD4 == 0) {
		pressed_column = 0;
		return;
	}
	pressed_column = NOT_PRESSED;
}