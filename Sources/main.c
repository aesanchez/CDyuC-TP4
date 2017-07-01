#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "keyboard.h"
#include "ledcontroller.h"

#ifdef __cplusplus
extern "C"
#endif
void MCU_init(void); /* Device initialization function declaration */

void main(void) {
	MCU_init();
	//TODO dejar esto aca? o hacer algo como RTC_init();
	RTCSC=0x88;//RTC cada 1 ms con las interrupciones apagadas
	keyboard_init();
	ledcontroller_init();
	for (;;) {
		ledcontroller_run();
	}
}
