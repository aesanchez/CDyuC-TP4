#include <mc9s08sh8.h>
#include "keyevent.h"
#include "keyboard.h"

#define NOT_PRESSED 42 //meaning of life
const char EMPTY_KEY = ' ';
char last_pressed_key;
char pressed_row;
char pressed_column;

const char keymap[4][4] = { { '1', '2', '3', 'A' },
                         { '4', '5', '6', 'B' },
                         { '7',	'8', '9', 'C' },
                         { '*', '0', '#', 'D' } };


void check_column(void);
void check_row(void);

void keyboard_init(void){
	PTBDD = 0x0F;
	PTBPE = 0xF0;
	last_pressed_key=EMPTY_KEY;
}

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
