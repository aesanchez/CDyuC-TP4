#ifndef LEDCONTROLLER_H_
#define LEDCONTROLLER_H_

void ledcontroller_init(void);
void ledcontroller_run(void);
void ledcontroller_interrupt_handler(void);
void ledcontroller_intensity_up();
void ledcontroller_intensity_down();

#endif