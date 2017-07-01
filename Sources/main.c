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
	//TODO dejar esto aca? o hacer algo como RTC_init();
	RTCSC=0x88;//RTC cada 1 ms con las interrupciones apagadas
	keyboard_init();
	potentiometer_init();
	ledcontroller_init();
	RTCS_RTIE=1;//arrancar la cuestion cuando este todo preparado
	//TODO RTC_start()???
	for (;;) {
		ledcontroller_run();
	}
}
