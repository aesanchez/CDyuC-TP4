#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "keyboard.h"
#include "ledcontroller.h"
#include "potentiometer.h"

#ifdef __cplusplus
extern "C"
#endif
void MCU_init(void); /* Device initialization function declaration */

void main(void) {
	MCU_init();
	// Inicializamos el Real Time Counter para que se realiza una interrupcion cada 1 ms
	// En particular, se inicializa con las interrupciones apagadas.
	RTCSC=0x88;

	// Inicializamos los modulos del sistema
	keyboard_init();
	potentiometer_init();
	ledcontroller_init();

	// Habilitamos la interrupcion del RTC, para dar comienzo al sistema
	for (;;) {
		// Iteramos indefinidamente llamando a ledcontroller_run para permitir
		// el funcionamiento constante del controlador del led. 
		ledcontroller_run();
	}
}
