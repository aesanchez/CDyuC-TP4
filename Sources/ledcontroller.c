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

#define RED 0
#define GREEN 1
#define BLUE 2

typedef enum{OFF,NORMAL,BLINKING,SWEEPING} state;
state current_state;

typedef struct led_type{
    char duty_cycle;
    char cycle_iteration;
    char state;
    char previous_state;//para el blink
    char mask;//mascara para acceder al valor
}
led_type rgb[3];

//variables mundanas
static char key;
char led_intensity;//intensidad total del sistema
int blink_period = 700;
unsigned int blink_counter;
char unsigned blink_state;//1->ON
unsigned char sweep_counter;
char sweep_next_color;

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
    if(current_state==OFF){
        if(key!='*') return; //ignorar todo menos ON
        state_on();
    }else{
        if(key=='*') return; //ignorar los ON
        switch(key){
            case '#': state_off();      break;
            case 'A': blink_vel_up();   break;
            case 'B': blink_vel_down(); break;
            case 'C': blink_toggle();   break;
            case 'D': sweep_toggle();   break;
            case '0': set_white();      break;
            case:'1':
            case:'2':
            case:'3': led_toggle_state((key-'0') % 3); break;
            case:'4':
            case:'5':
            case:'6': led_intensity_up((key-'0') % 3); break;
            case:'7':
            case:'8':
            case:'9': led_intensity_down((key-'0') % 3);break;
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
    //no los desactivamos cosa de que cuando se vuelve a prender se mantiene le estado que estaba antes
    led_off(RED);
    led_off(BLUE);
    led_off(GREEN);
}

void set_white(void){
    current_state=NORMAL;
    rgb[RED].duty_cycle=MAX_CYCLE;
    rgb[GREEN].duty_cycle=MAX_CYCLE;
    rgb[BLUE].duty_cycle=MAX_CYCLE;
    led_activate(RED);
    led_activate(GREEN);
    led_activate(BLUE);
}

void ledcontroller_init(void){
    current_state=OFF;
    //configurar los puertos como salida
    PTCDD_PTCDD1=1;
    PTCDD_PTCDD2=1;
    PTCDD_PTCDD3=1;
    //las mascaras del bit que corresponden en PTC
    rgb[RED].mask=PTCD_PTCD1_MASK;
    rgb[GREEN].mask=PTCD_PTCD2_MASK;
    rgb[BLUE].mask=PTCD_PTCD3_MASK;
    //que arranquen prendidos todos
    rgb[RED].duty_cycle=MAX_DUTY_CYCLE;
    rgb[GREEN].duty_cycle=MAX_DUTY_CYCLE;
    rgb[BLUE].duty_cycle=MAX_DUTY_CYCLE;
    //los activamos//se podria hacer led_activate();
    rgb[RED].state=1;
    rgb[GREEN].state=1;
    rgb[BLUE].state=1;
    //led_intensity=INTENSITY_SCALE;
    //arranca fully bright? el tema es que de toque
    // se va a llamar a la funcion para actualizar esto y no nos cambia en nada
}

void led_desactivate(char led_index){
    rgb[led_index].state=0;
    led_off(led_index);//forzar la salida para que se apague    
}

void led_activate(char led_index){
    rgb[led_index].state=1;
    //no hace falta forzar a "prendido" porque el mismo pwm lo va a prender
}

void led_toggle_state(char led_index){
    if(rgb[led_index].state) led_desactivate(led_index);
    else led_activate(led_index);
}

void led_on(char led_index){
    //led on=0
    PTCD= PTCD & (~rgb[led_index].mask)
    /*
    PTCD  xxxxxxxx
    and
    !mask 11111101
    --------------
    resul xxxxxx0x
    */    
}

void led_off(char led_index){
    //led off=1
    PTCD = PTCD | rgb[led_index].mask;
    /*
    PTCD  xxxxxxxx
    or
    mask  00000010
    --------------
    resul xxxxxx1x
    */
}

void led_intensity_up(char led_index){
    if(!rgb[led_index].state) return; //do nothing
    if(rgb[led_index].duty_cycle==MAX_DUTY_CYCLE) return;
    rgb[led_index].duty_cycle++;
}

void led_intensity_down(char led_index){
    if(!rgb[led_index].state) return; //do nothing
    if(rgb[led_index].duty_cycle==MIN_DUTY_CYCLE) return;
    rgb[led_index].duty_cycle--;
}

void ledcontroller_interrupt_handler(void){//llamada cada 1 ms
    if(rgb[RED].state)ledcontroller_pwm_handler(RED);
    if(rgb[GREEN].state)ledcontroller_pwm_handler(GREEN);
    if(rgb[BLUE].state)ledcontroller_pwm_handler(BLUE);
    if(current_state==SWEEPING)ledcontroller_sweep_handler();
    if(current_state==BLINKING)ledcontroller_blink_handler();
}

void ledcontroller_pwm_handler(char led_index){
    char true_intensity=(rgb[led_index].duty_cycle*led_intensity)/INTENSITY_SCALE;
    if(rgb[led_index].cycle_iteration<(PWM_PERIOD/INTERRUPT_PERIOD-true_intensity))
        led_off(led_index);
    else 
        led_on(led_index);
    rgb[led_index].cycle_iteration++;
    if(rgb[led_index].cycle_iteration==PWM_PERIOD/INTERRUPT_PERIOD) rgb[led_index].cycle_iteration=0;          
}

void ledcontroller_set_intensity(char intensity){
    led_intensity=intensity;
}

void ledcontroller_blink_handler(void){
    if(blink_counter++<(blink_period/INTERRUPT_PERIOD))return;
    blink_counter=0;
    if(blink_state){
        blink_state=0;
        rgb[RED].previous_state=rgb[RED].state;
        rgb[GREEN].previous_state=rgb[GREEN].state;
        rgb[BLUE].previous_state=rgb[BLUE].state;
        //desactiva todos, hasta los ya desactivados porque me daba paja poner ifs
        led_desactivate(RED);
        led_desactivate(GREEN);
        led_desactivate(BLUE);
    }else{
        blink_state=1;
        //activar solo los que estaban prendido previamente
        if(rgb[RED].previous_state)led_activate(RED);
        if(rgb[GREEN].previous_state)led_activate(GREEN);
        if(rgb[BLUE].previous_state)led_desactivate(BLUE);
    } 
}
void blink_toggle(void){
    if(current_state!=BLINKING){
        current_state=BLINKING;
        //init blink
        blink_counter=0;
        blink_state=1;
        
    }else{
        current_state=NORMAL;
    }
}
void blink_vel_down(void){
    if(blink_period==BLINK_MAX)return;
    blink_period+=BLINK_STEP;
}
void blink_vel_up(void){
    if(blink_period==BLINK_MIN)return;
    blink_period-=BLINK_STEP;
}

void sweep_toggle(void){
    if(current_state!=SWEEPING){
        current_state=SWEEPING;
        //init sweep
        sweep_counter=0;
        sweep_next_color=RED;
        rgb[RED].previous_state=rgb[RED].state;
        rgb[GREEN].previous_state=rgb[GREEN].state;
        rgb[BLUE].previous_state=rgb[BLUE].state;        
    }else{
        current_state=NORMAL;
    }
}

void ledcontroller_sweep_handler(void){
    if(sweep_counter++<(SWEEP_PERIOD/INTERRUPT_PERIOD))return;
    sweep_counter=0;
    char k;
    for(k=0;k<3;k++){
        if(k==sweep_next_color){
            //activar solo los que estaban prendido previamente
            if(rgb[k].previous_state)led_activate(k);
        }else{
            rgb[k].previous_state=rgb[k].state;
            led_desactivate(k);
        }
    }
    sweep_next_color=(sweep_next_color+1) % 3;
}

char ledcontroller_is_on(void){
    return current_state!=OFF;
}