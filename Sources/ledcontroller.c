#include <mc9s08sh8.h>
#include "ledcontroller.h"
#include "keyevent.h"
float led_intensity;
char system_state;
#define SYSTEM_ON 1
#define SYSTEM_OFF 0

//START: de lo relacionado a la toma de decisiones
#define NUMBER_OF_COMMANDS 16
const char key_association[NUMBER_OF_COMMANDS]= {'*','#','1','2','3','4','7','5','8','6','9','A','B','C','D','0'};
const char on_command='*';

void fON(void);
void fOFF(void);
void fRED_TOGGLE(void);
void fGREEN_TOGGLE(void);
void fBLUE_TOGGLE(void);
void fRED_UP(void);
void fRED_DOWN(void);
void fGREEN_UP(void);
void fGREEN_DOWN(void);
void fBLUE_UP(void);
void fBLUE_DOWN(void);
void fVEL_UP(void);
void fVEL_DOWN(void);
void fBLINK_TOGGLE(void);
void fSWEEP_TOGGLE(void);
void fWHITE(void);

void (*COMMANDS_FUNC[])(void) = {fON,fOFF,fRED_TOGGLE,fGREEN_TOGGLE,fBLUE_TOGGLE,fRED_UP,
                    fRED_DOWN,fGREEN_UP,fGREEN_DOWN,fBLUE_UP,fBLUE_DOWN,fVEL_UP,fVEL_DOWN,
                    fBLINK_TOGGLE,fSWEEP_TOGGLE,fWHITE};

static char key;
char key_aux;

void ledcontroller_run(){
    if(keyevent_is_empty())return;
    key=keyevent_pop();
    if(SYSTEM_OFF==system_state && key!=on_command) return;//solo se puede despertar del estado off con el commando de prender
    for(key_aux=0;key_aux<NUMBER_OF_COMMANDS;key_aux++){
        if(key==key_association[key_aux]) break;
    }    
    if(key_aux==NUMBER_OF_COMMANDS)return;//ERROR
    (*COMMANDS_FUNC[key_aux])();
}
//END: de lo relacionado a la toma de decisiones

//LEDs turn on with LOW
#define LED_ON 0
#define LED_OFF 1
//LEDs enable (direction I/O)
#define LED_ENABLE 1
#define LED_DISABLE 0



struct led {
   char     duty_cycle; //leer state
   char     state; // sirve para saber si el led estaba anteriormente prendido cuando se ejecuta el fON(), cosa de mantener el color que estaba.
   //char*    port; //TODO: no se si es de tipo char
   //char*    port_enable; //TODO
   char     cycle_iteration;  
};
#define RED_PORT PTCD_PTCD1
#define RED_ENABLE PTCDD_PTCDD1
#define GREEN_PORT PTCD_PTCD2
#define GREEN_ENABLE PTCDD_PTCDD2
#define BLUE_PORT PTCD_PTCD3
#define BLUE_ENABLE PTCDD_PTCDD3



struct led red;
struct led green;
struct led blue;

//states
char blink_on; unsigned int blink_counter;
char unsigned blink_state;//1->ON
char sweep_on; unsigned char sweep_counter;


//from 0(OFF) to 10(Completely on)
#define MAX_CYCLE 10
#define MIN_CYCLE 0

void ledcontroller_pwm_handler_red(void);
void ledcontroller_pwm_handler_green(void);
void ledcontroller_pwm_handler_blue(void);
void ledcontroller_sweep_handler(void);
void ledcontroller_blink_handler(void);

void ledcontroller_init(){
    //puertos como salida
    RED_ENABLE=LED_ENABLE;
    GREEN_ENABLE=LED_ENABLE;
    BLUE_ENABLE=LED_ENABLE;

    red.duty_cycle=MIN_CYCLE;
    green.duty_cycle=MIN_CYCLE;
    blue.duty_cycle=MIN_CYCLE;

    red.state=1;
    green.state=1;
    blue.state=1;

    RED_PORT=LED_OFF;
    GREEN_PORT=LED_OFF;
    BLUE_PORT=LED_OFF;

    red.cycle_iteration=0;
    green.cycle_iteration=0;
    blue.cycle_iteration=0;

    blink_on=0;
    sweep_on=0;
    system_state=SYSTEM_OFF;
    led_intensity=1.0;
    //TODO: configurar el RTC?
    RTCSC_RTIE=1;    
}

// led frequency should by around 100 Hz --> period of 10 ms
// if we want 0(OFF)-10(FULLY ON) levels of intensity --> interrupt every 1ms
//si asumimos que se llama cada 1 ms
#define PWM_PERIOD 10
#define SWEEP_PERIOD 500
#define BLINK_MAX 1000
#define BLINK_MIN 100
#define BLINK_STEP 300
int blink_period = 700;
#define INTERRUPT_PERIOD 1

void ledcontroller_interrupt_handler(void){//llamada cada 1 ms
    if(red.state)ledcontroller_pwm_handler_red();
    if(green.state)ledcontroller_pwm_handler_green();
    if(blue.state)ledcontroller_pwm_handler_blue();
    if(sweep_on)ledcontroller_sweep_handler();
    if(blink_on)ledcontroller_blink_handler();
}

// para prender y apagar los leds simplemente le cambiamos si son de entrada o salida
// cosa de no alterar todos los estados que tenian
// creeria que funciona
void fON(void){
    if(red.state) RED_ENABLE=LED_ENABLE;
    if(green.state) GREEN_ENABLE=LED_ENABLE;
    if(blue.state) BLUE_ENABLE=LED_ENABLE;
    system_state=SYSTEM_ON;
    //TODO: prender RTC?
}

void fOFF(void){
    RED_ENABLE=LED_DISABLE;
    GREEN_ENABLE=LED_DISABLE;
    BLUE_ENABLE=LED_DISABLE;
    system_state=SYSTEM_OFF;
    blink_on=0;
    sweep_on=0;
    //TODO: Apagar RTC? para que los interrupts no sigan??
    //solo apagaria el led, toda la logica de background sigue funcionando
}

void fRED_TOGGLE(void){    
    if(red.state){//estaba prendido
        red.state=0;
        RED_ENABLE=LED_DISABLE;
    }else{//estaba apagado
        red.state=1;
        RED_ENABLE=LED_ENABLE;
    }
}
void fGREEN_TOGGLE(void){
    if(green.state){//estaba prendido
        green.state=0;
        GREEN_ENABLE=LED_DISABLE;
    }else{//estaba apagado
        green.state=1;
        GREEN_ENABLE=LED_ENABLE;
    }    
}
void fBLUE_TOGGLE(void){
    if(blue.state){//estaba prendido
        blue.state=0;
        BLUE_ENABLE=LED_DISABLE;
    }else{//estaba apagado
        blue.state=1;
        BLUE_ENABLE=LED_ENABLE;
    }
}

void fRED_UP(void){
    if(!red.state) return; //do nothing
    if(red.duty_cycle==MAX_CYCLE)return;
    red.duty_cycle++;
}
void fRED_DOWN(void){
    if(!red.state) return; //do nothing
    if(red.duty_cycle==MIN_CYCLE)return;
    red.duty_cycle--;
}
void fGREEN_UP(void){
    if(!green.state) return; //do nothing
    if(green.duty_cycle==MAX_CYCLE)return;
    green.duty_cycle++;
}
void fGREEN_DOWN(void){
    if(!green.state) return; //do nothing
    if(green.duty_cycle==MIN_CYCLE)return;
    green.duty_cycle--;
}
void fBLUE_UP(void){
    if(!blue.state) return; //do nothing
    if(blue.duty_cycle==MAX_CYCLE)return;
    blue.duty_cycle++;
}
void fBLUE_DOWN(void){
    if(!blue.state) return; //do nothing
    if(blue.duty_cycle==MIN_CYCLE)return;
    blue.duty_cycle--;
}

void fVEL_DOWN(void){
    if(blink_period==BLINK_MAX)return;
    blink_period+=BLINK_STEP;
}
void fVEL_UP(void){
    if(blink_period==BLINK_MIN)return;
    blink_period-=BLINK_STEP;
}

void fBLINK_TOGGLE(void){  
    if (blink_on) blink_on=0;
    else{
        blink_on=1;
        blink_counter=0;
        blink_state=1;      
    }  
}
void fSWEEP_TOGGLE(void){   
    if (sweep_on) sweep_on=0;
    else{
        sweep_on=1;
        sweep_counter=0;
        //TODO
    } 
}

void fWHITE(void){
    sweep_on=0;
    blink_on=0;
    red.duty_cycle=MAX_CYCLE;
    green.duty_cycle=MAX_CYCLE;
    blue.duty_cycle=MAX_CYCLE;
    red.state=1;
    green.state=1;
    blue.state=1;
    RED_ENABLE=LED_ENABLE;
    GREEN_ENABLE=LED_ENABLE;
    BLUE_ENABLE=LED_ENABLE;
}

void ledcontroller_pwm_handler_red(void){
    if(red.cycle_iteration<(PWM_PERIOD/INTERRUPT_PERIOD-red.duty_cycle))
        RED_PORT=LED_OFF;
    else 
        RED_PORT=LED_ON;
    red.cycle_iteration++;
    if(red.cycle_iteration==PWM_PERIOD/INTERRUPT_PERIOD) red.cycle_iteration=0;          
}
void ledcontroller_pwm_handler_green(void){
    if(green.cycle_iteration<(PWM_PERIOD/INTERRUPT_PERIOD-green.duty_cycle))
        GREEN_PORT=LED_OFF;
    else 
        GREEN_PORT=LED_ON;
    green.cycle_iteration++;
    if(green.cycle_iteration==PWM_PERIOD/INTERRUPT_PERIOD) green.cycle_iteration=0;          
}
void ledcontroller_pwm_handler_blue(void){
    if(blue.cycle_iteration<(PWM_PERIOD/INTERRUPT_PERIOD-blue.duty_cycle))
        BLUE_PORT=LED_OFF;
    else 
        BLUE_PORT=LED_ON;
    blue.cycle_iteration++;
    if(blue.cycle_iteration==PWM_PERIOD/INTERRUPT_PERIOD) blue.cycle_iteration=0;          
}
void ledcontroller_sweep_handler(void){
    if(sweep_counter++<(SWEEP_PERIOD/INTERRUPT_PERIOD))return;
    //TODO barrido
}

void ledcontroller_blink_handler(void){
    if(blink_counter++<(blink_period/INTERRUPT_PERIOD))return;
    blink_counter=0;
    if(blink_state){
        blink_state=0;
        RED_ENABLE=LED_DISABLE;
        GREEN_ENABLE=LED_DISABLE;
        BLUE_ENABLE=LED_DISABLE; 
    }else{
        blink_state=1;
        fON();
    } 
}

void ledcontroller_set_intensity(float intensity){
    led_intensity=intensity;
    red.duty_cycle=(char)(red.duty_cycle*led_intensity);
    green.duty_cycle=(char)(green.duty_cycle*led_intensity);
    blue.duty_cycle=(char)(blue.duty_cycle*led_intensity);
}

