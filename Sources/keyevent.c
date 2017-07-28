#include "keyevent.h"

// key almacena el evento y actua como un buffer de un solo caracter
static char key;

// empty indica si hay un evento(tecla presionada) en el buffer que deberia ser consumido.
// empty=1 indica que esta vacio, que no hay evento.
char empty = 1;

// keyevent_push almacena la tecla c pasa por parametro, en el buffer
void keyevent_push(char c) {
	key = c;
	empty = 0;
}

// keyevent_pop retira del bufffer y devuelve la ultima tecla presionada
char keyevent_pop() {
	empty = 1;
	return key;
}

// leyevent_is_empty devuelve 1 si no hay ningun evento para consumir.
// Devuelve 0, en caso contrario
char keyevent_is_empty() {
	return empty;
}
