#ifndef LEDCONTROLLER_H_
#define LEDCONTROLLER_H_

// INTENSITY_SCALE exporta la escala en que se trabajo la intensidad maxima del sistema.
// 10: 100%
// 0 :  0%
#define INTENSITY_SCALE 10

// ledcontroller_init se encarga de las configuraciones iniciales de ledcontroller. Inicializa
// los puertos en que se conecta cada led (PTC pines 1,2 y 3) e inicializa todas las variables
// internas necesarias. Se empieza en el estado apagado.
void ledcontroller_init(void);

// ledcontroller_run es el encargado de leer los eventos de teclas presionadas.
// En base a la tecla presionada y si el ledcontroller esta apagado o no, deriva 
// la accion a sus funcion interna correspondiente
void ledcontroller_run(void);

// ledcontroller_interrupt_handler funcion que es llamada cada INTERRUPT_PERIOD, osea 1 ms en el caso normal.
// Se encarga de los llamados a las funciones del control del PWM de cada led, del barrido y del blinkeo,
// segun corresponda.
void ledcontroller_interrupt_handler(void);

// ledcontroller_set_intensity establece la intensidad maxima recibida por parametro. El parametro
// debe ser coherente a la escala de la intensidad dada por INTENSITY_SCALE
void ledcontroller_set_intensity(char);

// ledcontroler_is_on devuelve si el estado del ledcontroller esta apagado o prendido. Permite la validacion
// de si se debe llamar ledcontroller_interrupt_handler en caso de que el sistema este prendido.
// 1: prendido(NORMAL,BLINKING o SWEEPING)
// 0: apagado(OFF)
char ledcontroller_is_on(void);

#endif
