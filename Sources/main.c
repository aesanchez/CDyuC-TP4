#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "interpreter.h"
#include "keyboard.h"

#ifdef __cplusplus
extern "C"
#endif
void MCU_init(void); /* Device initialization function declaration */

void main(void) {
	MCU_init();
	interpreter_init();
	keyboard_init();
	for (;;) {
		interpreter_run();
	}
}
