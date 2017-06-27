#include "potentiometer.h"
#include "mc9s08sh8.h"
#include "ledcontroller.h"
#define MAX_VALUE 1023

void potentiometer_init(){
    //TODOconfiguraciones del potenciomentro que hicimos en el MCUINIT
}

void potentiometer_interrupt_handler(){
    //calcular intensidad
    //  0<  valor_sensor<1023
    //  0<  intensidad  <10
    ledcontroller_set_intensity((ADCR*10)/1023);
}
