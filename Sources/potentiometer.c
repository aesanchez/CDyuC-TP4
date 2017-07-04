#include "potentiometer.h"
#include "mc9s08sh8.h"
#include "ledcontroller.h"
#define MAX_VALUE 1023

void potentiometer_init(){
    //bus clock 8Mhz - modo de conversion continuo - data format 10 bits - pin=PTA2/ADP2 - inicializacion channel 2
    APCTL1=0x00U;
    ADCCFG=0x08U;
    ADCCV=0x00U;
    ADCSC2=0x00U;
    ADCSC1=0x22U;
}

void potentiometer_interrupt_handler(){
    //calcular intensidad
    //  0<  valor_sensor<1023
    //  0<  intensidad  <MAX_INTENSITY
    ledcontroller_set_intensity((ADCR*INTENSITY_SCALE)/MAX_VALUE);
}