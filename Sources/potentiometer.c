#include "potentiometer.h"
#include "ledcontroller.h"

void potentiometer_init(){
    //conectado al ADP2
    //adp2=PTA2

    //aca abajo listo todos los tipos de inicializaciones

    ADCCFG =0x18; // enable long sampling, 10-bit mode
    APCTL1_ADPC0=1; // ADP0 in analog mode
    ADCSC1 = 0x20; // ADC in continuous mode, channel 0
    // si ADCSC1_COCO==1 termino la conversion
    ------------------------------
    ADCSC1_ADCH=0x00010; //adp2
    -----------------------------
    // Enable ACMP, ACMP interrupts,
    //ACF is set on falling edges of ACO
    ACMPSC = 0x90;
    -------------------------
    ACMPSC_ACME=1; // Enable ACMP
    ----------------
    // enable long sampling, 10-bit mode, ADICLK = 11b, ADCK = BUSCLK/2
    ADCCFG = ADLSMP | ADC_10BITS | ADC_BUSCLK_DIV2 | ADIV_8;
    APCTL1 = 0x03; // ADP0 and ADP1 in analog mode
    // ADC channel 0, interrupts enabled
    ADCSC1_ AIEN=1;
    ADCSC1_ADCH=0x00000; // channel = 0
    
}

_interrupt void isrVadc (void){
    if (ADCSC1_ADCH) // if ADCH=1? // esto pregunta el chanel que genero la interrupcion
    {
        sensor = ADCR; // read the channel 1 result
        ADCSC1_ADCH=0x00000; // next channel = 0
    }else{
        trimpot = ADCR; // read the channel 0 result
        ADCSC1_ADCH=0x00001; // next channel = 1
    }
}



//toda la logica del potenciomentro va aca, tambien va a tener u llamado de interrupcion
void potentiometer_interrupt_handler(){
    //ni idea que es lo que generaria el interrupt
    //creo que es el finalizar la comparacion
    //pero el valor se leeria en ADCR y es entre 0 y 1023
    result = ADCR;//read the result
}

//procesada la interrupcion la interaccion con el led seria algo asi
#include "ledcontroller.h"

void intensity_up(){
    ledcontroller_intensity_up();
}
void intensity_down(){
    ledcontroller_intensity_down();
}