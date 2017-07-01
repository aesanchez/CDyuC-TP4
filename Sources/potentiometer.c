#include "potentiometer.h"
#include "mc9s08sh8.h"
#include "ledcontroller.h"
#define MAX_VALUE 1023

void potentiometer_init(){
    //TODO configuraciones del potenciomentro que hicimos en el MCUINIT
    //ver que cambia cuando hacemos el device initialization y meterlo aca
}

void potentiometer_interrupt_handler(){
    //calcular intensidad
    //  0<  valor_sensor<1023
    //  0<  intensidad  <MAX_INTENSITY
    ledcontroller_set_intensity((ADCR*INTENSITY_SCALE)/MAX_VALUE);
}