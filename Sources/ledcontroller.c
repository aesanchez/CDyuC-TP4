#include "ledcontroller.h"
#include "keyevent.h"

//START: de lo relacionado a la toma de decisiones
const char * key_association= {'*','#','1','2','3','4','7','5','8','6','9','A','B','C','D','0'};
const char NUMBER_OF_COMMANDS= 16;

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

char key;
char key_aux;

void ledcontroller_run(){
    if(keyevent_is_empty())return;
    key=keyevent_pop;
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
   char     duty_cycle;
   char     state;
   char*    port; //TODO: no se si es de tipo char
   char*    port_enable; //TODO
   char     cycle_iteration;  
};
struct led red;
struct led green;
struct led blue;

//states
char blink_on; unsigned char blink_counter;
char unsigned blink_state;//1->ON
char sweep_on; unsigned char sweep_counter;


//from 0(OFF) to 5(Completely on)
#define MAX_CYCLE 5
#define MIN_CYCLE 0

void ledcontroller_pwm_handler(struct led);
void ledcontroller_sweep_handler(void);
void ledcontroller_blink_handler(void);

void ledcontroller_init(){
    //puertos como salida
    red.port_enable=&PTCDD_PTCDD1;
    *red.port_enable=LED_ENABLE;
    green.port_enable=&PTCDD_PTCDD2;
    *green.port_enable=LED_ENABLE;
    blue.port_enable=&PTCDD_PTCDD3;
    *blue.port_enable=LED_ENABLE;

    red.duty_cycle=MIN_CYCLE;
    green.duty_cycle=MIN_CYCLE;
    blue.duty_cycle=MIN_CYCLE;

    red.state=1;
    green.state=1;
    red.state=1;

    red.port=&PTCD_PTCD1;
    green.port=&PTCD_PTCD2;
    blue.port=&PTCD_PTCD3;

    *red.port=LED_OFF;
    *green.port=LED_OFF;
    *blue.port=LED_OFF;

    red.cycle_iteration=0;
    green.cycle_iteration=0;
    blue.cycle_iteration=0;

    blink_on=0;
    sweep_on=0;

    //TODO: configurar el RTC?    
}

// led frequency should by around 100 Hz --> period of 10 ms
// if we want 6(0(OFF),1,2,3,4,5(FULL)) levels of intensity --> interrupt every 2ms
//si asumimos que se llama cada 2 ms
#define PWM_PERIOD 10
#define SWEEP_PERIOD 500
#define BLINK_PERIOD 250
#define INTERRUPT_PERIOD 2

void ledcontroller_interrupt_handler(void){//llamada cada 2 ms
    if(red.state)ledcontroller_pwm_handler(red);
    if(green.state)ledcontroller_pwm_handler(green);
    if(blue.state)ledcontroller_pwm_handler(blue);
    if(sweep_on)ledcontroller_sweep_handler();
    if(blink_on)ledcontroller_blink_handler();
}

// para prender y apagar los leds simplemente le cambiamos si son de entrada o salida
// cosa de no alterar todos los estados que tenian
// creeria que funciona
void fON(void){
    if(red.state) *red.port_enable=LED_ENABLE;
    if(green.state) *green.port_enable=LED_ENABLE;
    if(blue.state) *blue.port_enable=LED_ENABLE;
    //TODO: prender RTC?
}

void fOFF(void){
    *red.port_enable=LED_DISABLE;
    *green.port_enable=LED_DISABLE;
    *blue.port_enable=LED_DISABLE;
    //TODO: Apagar RTC? para que los interrupts no sigan??
    //solo apagaria el led, toda la logica de background sigue funcionando
}

void fRED_TOGGLE(void){    
    if(red.state){//estaba prendido
        red.state=0;
        *red.port_enable==LED_DISABLE;
    }else{//estaba apagado
        red.state=1;
        *red.port_enable==LED_ENABLE;
    }
}
void fGREEN_TOGGLE(void){
    if(green.state){//estaba prendido
        green.state=0;
        *green.port_enable==LED_DISABLE;
    }else{//estaba apagado
        green.state=1;
        *green.port_enable==LED_ENABLE;
    }    
}
void fBLUE_TOGGLE(void){
    if(blue.state){//estaba prendido
        blue.state=0;
        *blue.port_enable==LED_DISABLE;
    }else{//estaba apagado
        blue.state=1;
        *blue.port_enable==LED_ENABLE;
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

void fVEL_UP(void){
    //TODO
}
void fVEL_DOWN(void){
    //TODO
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
}

void ledcontroller_pwm_handler(struct led l){
    if(l.cycle_iteration<(PWM_PERIOD/INTERRUPT_PERIOD-l.duty_cycle))
        *l.port=LED_OFF;
    else 
        *l.port=LED_ON;
    l.cycle_iteration++;
    if(l.cycle_iteration==PWM_PERIOD/INTERRUPT_PERIOD) l.cycle_iteration=0;          
}
void ledcontroller_sweep_handler(void){
    if(sweep_counter++<(SWEEP_PERIOD/INTERRUPT_PERIOD))return;
    //TODO barrido
}

void ledcontroller_blink_handler(void){
    if(blink_counter++<(BLINK_PERIOD/INTERRUPT_PERIOD))return;
    blink_counter=0;
    if(blink_state) fOFF();
    else fON();
    blink_state=(blink_state+1)%2;//toggle
}