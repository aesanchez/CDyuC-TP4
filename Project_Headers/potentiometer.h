#ifndef POTENTIOMETER_H_
#define POTENTIOMETER_H_

#define POTENTIOMETER_PERIOD 25 //ms

void potentiometer_init(void);
void potentiometer_interrupt_handler(void);

#endif