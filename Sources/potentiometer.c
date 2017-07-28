#include "potentiometer.h"
#include "mc9s08sh8.h"
#include "ledcontroller.h"

// MAX_VALUE indica el valor maximo que se puede leer del potenciometro,
// correspondiente a las configuraciones del conversor analogico-digital
#define MAX_VALUE 1023

// potentiometer_init encargado de inicializar con la configuracion correspondiente
// al ADC integrado
void potentiometer_init(){
    // bus clock 8Mhz
    // modo de conversion continuo
    // data format 10 bits
    // pin=PTA2/ADP2
    // inicializacion channel 2
    APCTL1=0x00U;
    ADCCFG=0x08U;
    ADCCV=0x00U;
    ADCSC2=0x00U;
    ADCSC1=0x22U;
}

// potentiometer_interrupt_handler es el encargado de leer el valor del registro del
// conversor analogico-digital y actualizar la intensidad de ledcontroller en la escala
// correspondiente
void potentiometer_interrupt_handler(){
    // calcular intensidad
    // 0 < ADCR (valor sensor) < MAX_VALUE
    // 0 <   intensidad led    < INTENSITY_SCALE
    // donde ITENSITY_SCALE es un valor de escala exportado por ledcontroller
    ledcontroller_set_intensity((ADCR*INTENSITY_SCALE)/MAX_VALUE);
}