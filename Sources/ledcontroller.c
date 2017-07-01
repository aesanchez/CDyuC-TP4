#include <mc9s08sh8.h>
#include "ledcontroller.h"
#include "keyevent.h"

//todos los periodos definidos en ms
#define INTERRUPT_PERIOD 1

#define PWM_PERIOD 10
#define MAX_DUTY_CYCLE (PWM_PERIOD/INTERRUPT_PERIOD)
#define MIN_DUTY_CYCLE 0

#define SWEEP_PERIOD 200 

#define BLINK_MAX 1000 
#define BLINK_MIN 100
#define BLINK_STEP 300

typedef enum{OFF,NORMAL,BLINKING,SWEEPING} state;
state current_state;

//variables mundanas
static char key;
char led_intensity;//intensidad total del sistema
int blink_period = 700;
unsigned int blink_counter;
char unsigned blink_state;//1->ON
char aux;
unsigned char sweep_counter;

typedef struct led_type{
    char duty_cycle;
    char cycle_iteration;
    char state;
    char previous_state;//para el blink
    char mask;//mascara para acceder al valor
}

led_type red, green, blue;

/*
KEY MAP
=======
   *    --> state_on()
   #    --> state_off()
   A    --> blink_vel_up()
   B    --> blink_vel_down()
   C    --> blink_toggle()
   D    --> sweep_toggle()
   0    --> set_white()
 R/G/B
 1/2/3  --> led_toggle_state(led)
 4/5/6  --> led_intensity_up(led)
 7/8/9  --> led_intensity_down(led)
*/

void ledcontroller_run(){
    if(keyevent_is_empty())return;
    key=keyevent_pop();
    //llamada a maquina de estados???
    if(current_state==OFF){
        //solo le interesa la tecla para prender el led
        if(key!='*') return; //ignorar todo menos ON
        state_on();
    }else{
        //leer todo menos la tecla on
        if(key=='*') return; //ignorar los ON
        //a lo tosco seria algo asi
        aux=(key-'0') mod 3; //me devuelve al led que corresponde 1/2/3
        switch(key){
            case '#': state_off();      break;
            case 'A': blink_vel_up();   break;
            case 'B': blink_vel_down(); break;
            case 'C': blink_toggle();   break;
            case 'D': sweep_toggle();   break;
            case '0': set_white();      break;
            case:'1':
            case:'2':
            case:'3': led_toggle_state(led pertinente); break;
            case:'4':
            case:'5':
            case:'6': led_intensity_up(led_pertinente); break;
            case:'7':
            case:'8':
            case:'9': led_intensity_down(del led que corresponda);break;
            default: /*error*/ break;
        }
    }
}

void state_on(void){
    current_state=NORMAL;
}

void state_off(void){
    current_state=OFF;
    //apagar forzadamente los leds por si estaban a la mitad de un pwm o algo
    led_off(red);
    led_off(blue);
    led_off(green);
}

void fWHITE(void){
    current_state=NORMAL;
    red.duty_cycle=MAX_CYCLE;
    green.duty_cycle=MAX_CYCLE;
    blue.duty_cycle=MAX_CYCLE;
    led_activate(red);
    led_activate(green);
    led_activate(blue);
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
    //que arranquen prendidos todos
    red.duty_cycle=MAX_DUTY_CYCLE;
    green.duty_cycle=MAX_DUTY_CYCLE;
    blue.duty_cycle=MAX_DUTY_CYCLE;
    //los activamos//se podria hacer led_activate();
    red.state=1;
    green.state=1;
    blue.state=1;
    //led_intensity=INTENSITY_SCALE;
    //arranca fully bright? el tema es que de toque
    // se va a llamar a la funcion para actualizar esto y no nos cambia en nada
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

void led_intensity_up(led_type led){
    if(!led.state) return; //do nothing
    if(led.duty_cycle==MAX_DUTY_CYCLE) return;
    led.duty_cycle++;
}

void led_intensity_down(led_type led){
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

void ledcontroller_blink_handler(void){
    if(blink_counter++<(blink_period/INTERRUPT_PERIOD))return;
    blink_counter=0;
    if(blink_state){
        blink_state=0;
        red.previous_state=red.state;
        green.previous_state=green.state;
        blue.previous_state=blue.state;
        //desactiva todos, hasta los ya desactivados porque me daba paja poner ifs
        led_desactivate(red);
        led_desactivate(green);
        led_desactivate(blue);
    }else{
        blink_state=1;
        //activar solo los que estaban prendido previamente
        if(red.previous_state)led_activate(red);
        if(green.previous_state)led_activate(green);
        if(blue.previous_state)led_desactivate(blue);
    } 
}
void fBLINK_TOGGLE(void){
    if(current_state!=BLINKING){
        current_state=BLINKING;
        blink_counter=0;
        blink_state=1;
    }else{
        current_state=NORMAL;
    }
}
void fVEL_DOWN(void){
    if(blink_period==BLINK_MAX)return;
    blink_period+=BLINK_STEP;
}
void fVEL_UP(void){
    if(blink_period==BLINK_MIN)return;
    blink_period-=BLINK_STEP;
}


//TODO REFACTOREAR
//se podria tener un arreglo de colores bien piola
char sweep_next_state;
void fSWEEP_TOGGLE(void){
    blink_on=0; 
    if (sweep_on) sweep_on=0;
    else{
        sweep_on=1;
        sweep_counter=0;
        sweep_next_state='R';
        //TODO
    } 
}

void ledcontroller_sweep_handler(void){
    if(sweep_counter++<(SWEEP_PERIOD/INTERRUPT_PERIOD))return;
    switch (sweep_next_state){
        case 'R':
        red.state=1;
        green.state=0;
        blue.state=0;
        RED_ENABLE=LED_ENABLE;
        GREEN_ENABLE=LED_DISABLE;
        BLUE_ENABLE=LED_DISABLE;
        sweep_next_state='G';
        break;
        case 'G':
        red.state=0;
        green.state=1;
        blue.state=0;
        RED_ENABLE=LED_DISABLE;
        GREEN_ENABLE=LED_ENABLE;
        BLUE_ENABLE=LED_DISABLE;
        sweep_next_state='B';
        break;
        case 'B':
        red.state=0;
        green.state=0;
        blue.state=1;
        RED_ENABLE=LED_DISABLE;
        GREEN_ENABLE=LED_DISABLE;
        BLUE_ENABLE=LED_ENABLE;
        sweep_next_state='R';
        break;
    }
}


