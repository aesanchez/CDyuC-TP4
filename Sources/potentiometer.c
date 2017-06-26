#include "potentiometer.h"

void potentiometer_init(){

}
//toda la logica del potenciomentro va aca, tambien va a tener u llamado de interrupcion
void potentiometer_interrupt_handler(){

}

//procesada la interrupcion la interaccion con el led seria algo asi
#include "ledcontroller.h"

void intensity_up(){
    ledcontroller_intensity_up();
}
void intensity_down(){
    ledcontroller_intensity_down();
}