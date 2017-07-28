#ifndef KEYEVENT_H_
#define KEYEVENT_H_
// KEYEVENT acutua como un buffer de eventos de teclas presionadas.
// En particular, como buffer de un solo caracter a la vez

// keyevent_push almacena la tecla c, pasa por parametro, en el buffer
void keyevent_push(char);

// keyevent_pop retira del bufffer y devuelve la ultima tecla presionada
char keyevent_pop(void);

// leyevent_is_empty devuelve 1 si no hay ningun evento para consumir.
// Devuelve 0, en caso contrario
char keyevent_is_empty(void);

#endif