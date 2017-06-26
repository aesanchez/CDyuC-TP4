#include "potentiometer.h"
#include "ledcontroller.h"

void potentiometer_init(){
    //conectado al ADP2
}
//toda la logica del potenciomentro va aca, tambien va a tener u llamado de interrupcion
void potentiometer_interrupt_handler(){
    //ni idea que es lo que generaria el interrupt
    //pero el valor se leeria en ADCR y es entre 0 y 1023
}

//procesada la interrupcion la interaccion con el led seria algo asi
#include "ledcontroller.h"

void intensity_up(){
    ledcontroller_intensity_up();
}
void intensity_down(){
    ledcontroller_intensity_down();
}