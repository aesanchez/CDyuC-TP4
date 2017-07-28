#ifndef KEYBOARD_H_
#define KEYBOARD_H_

// KEYBOARD_CHECK_PERIOD indica el periodo con la que se va a llamar
// a la rutina keyboard_check_key
#define KEYBOARD_CHECK_PERIOD 30

// keyboard_check_key se encarga de implementar el algoritmo de 
// identificacion de la tecla presionada. Si se detecta el presionado
// y liberado de una tecla se pushea el valor de la tecla al buffer
// keyevent.
void keyboard_check_key(void);

// keyboard_init inicializa las configuraciones del puerto B
// en el que esta conectado el teclado matricial
void keyboard_init(void);

#endif