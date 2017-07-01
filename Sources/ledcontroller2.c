#include <mc9s08sh8.h>
#include "ledcontroller.h"
#include "keyevent.h"

#define MAX_DUTY_CYCLE 10
#define MIN_DUTY_CYCLE 0

#define PWM_PERIOD 10

#define SWEEP_PERIOD 200

#define BLINK_MAX 1000
#define BLINK_MIN 100
#define BLINK_STEP 300

#define INTERRUPT_PERIOD 1

typedef enum{OFF,NORMAL,BLINKING,SWEEPING} state;
state current_state;

//variables mundanas
static char key;
char led_intensity;//intensidad total del sistema
int blink_period = 700;

typedef struct led_type{
    char duty_cycle;
    char cycle_iteration;
    char state;
    char mask;//mascara para acceder al valor
}

led_type red, green, blue;


void ledcontroller_run(){
    if(keyevent_is_empty())return;
    key=keyevent_pop();
    //llamada a maquina de estados???
    if(current_state==OFF){
        //solo le interesa la tecla para prender el led
        if(key!='*') return; //ignorar todo menos ON
        fON();
    }else{
        //leer todo menos la tecla on
        if(key=='*') return; //ignorar los ON
        //
        //llamar a lo que corresponda
        //
    }
}

void fON(void){
    current_state=NORMAL;
}

void fOFF(void){
    current_state=OFF;
    //apagar forzadamente los leds por si estaban a la mitad de un pwm o algo
    led_off(red);
    led_off(blue);
    led_off(green);
}

void ledcontroller_init(){
    current_state=OFF;
    //configurar los puertos como salida
    PTCDD_PTCDD1=1;
    PTCDD_PTCDD2=1;
    PTCDD_PTCDD3=1;
    //las mascaras del bit que corresponden en PTC
    red.mask=PTCD_PTCD1_MASK;
    green.mask=PTCD_PTCD2_MASK;
    blue.mask=PTCD_PTCD3_MASK;

}

void led_desactivate(led_type led){
    led.state=0;
    led_off(led);//forzar la salida para que se apague    
}

void led_activate(led_type led){
    led.state=1;
    //no hace falta forzar a "prendido" porque el mismo pwm lo va a prender
}

void led_toggle_state(led_type led){
    if(led.state) led_desactivate(led);
    else led_activate(led);
}

void led_on(led_type led){
    //led on=0
    PTCD= PTCD & (~led.mask)
    /*
    PTCD  xxxxxxxx
    and
    !mask 11111101
    --------------
    resul xxxxxx0x
    */    
}

void led_off(led_type led){
    //led off=1
    PTCD = PTCD | led.mask;
    /*
    PTCD  xxxxxxxx
    or
    mask  00000010
    --------------
    resul xxxxxx1x
    */
}

void led_more_intensity(led_type led){
    if(!led.state) return; //do nothing
    if(led.duty_cycle==MAX_DUTY_CYCLE) return;
    led.duty_cycle++;
}

void led_less_intensity(led_type led){
    if(!led.state) return; //do nothing
    if(led.duty_cycle==MIN_DUTY_CYCLE) return;
    led.duty_cycle--;
}

void ledcontroller_interrupt_handler(void){//llamada cada 1 ms
    if(current_state==OFF) return;
    if(red.state)ledcontroller_pwm_handler(red);
    if(green.state)ledcontroller_pwm_handler(green);
    if(blue.state)ledcontroller_pwm_handler(blue);
    if(current_state==SWEEPING)ledcontroller_sweep_handler();
    if(current_state==BLINKING)ledcontroller_blink_handler();
}

void ledcontroller_pwm_handler(led_type led){
    char true_intensity=(led.duty_cycle*led_intensity)/INTENSITY_SCALE;
    if(led.cycle_iteration<(PWM_PERIOD/INTERRUPT_PERIOD-true_intensity))
        led_off(led);
    else 
        led_on(led);
    led.cycle_iteration++;
    if(led.cycle_iteration==PWM_PERIOD/INTERRUPT_PERIOD) led.cycle_iteration=0;          
}

void ledcontroller_set_intensity(char intensity){
    led_intensity=intensity;
}