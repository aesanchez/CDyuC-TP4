#ifndef POTENTIOMETER_H_
#define POTENTIOMETER_H_

// POTENTIOMETER_PERIOD indica el periodo en milisegundos con la que se va a llamar
// a la potentiometer_interrupt_handler
#define POTENTIOMETER_PERIOD 25

// potentiometer_init encargado de inicializar con la configuracion correspondiente
// al ADC integrado
void potentiometer_init(void);

// potentiometer_interrupt_handler es el encargado de leer el valor del registro del
// conversor analogico-digital y actualizar la intensidad de ledcontroller en la escala
// correspondiente
void potentiometer_interrupt_handler(void);

#endif