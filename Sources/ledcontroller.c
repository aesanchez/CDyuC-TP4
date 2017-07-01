//TODO probablemente estaria bueno hacer estados tipo: on, off, sweep y blink para que las cosas sean mas faciles
//una cosa piola tambien seria en vez de cambiar la direccion de los pines cada vez que lo queremos pagar,
//apagarlos poniendo el duty cycle en 0/ o tambien apagar su respectivo pwm y forzar la salida a LED_ON
//de esa menera la inicializacion con los puertos PTCDD que indican la direccion solo se tocan en el init
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
    
    for(key_aux=0;key_aux<NUMBER_OF_COMMANDS;key_aux++){
        if(key==key_association[key_aux]) break;
    }    
    if(key_aux==NUMBER_OF_COMMANDS)return;//ERROR
    (*COMMANDS_FUNC[key_aux])();
}
//END: de lo relacionado a la toma de decisiones

//states
char blink_on; unsigned int blink_counter;
char unsigned blink_state;//1->ON
char sweep_on; unsigned char sweep_counter;

void ledcontroller_init(){
    //puertos como salida
    RED_ENABLE=LED_ENABLE;
    GREEN_ENABLE=LED_ENABLE;
    BLUE_ENABLE=LED_ENABLE;

    red.duty_cycle=(MAX_CYCLE-MIN_CYCLE)/2;
    green.duty_cycle=(MAX_CYCLE-MIN_CYCLE)/2;
    blue.duty_cycle=(MAX_CYCLE-MIN_CYCLE)/2;

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
    system_state=SYSTEM_ON;
    led_intensity=1.0; 
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
    sweep_on=0;
    if (blink_on) blink_on=0;
    else{
        blink_on=1;
        blink_counter=0;
        blink_state=1;

    }  
}
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
    char cycle_intensity=(red.duty_cycle*led_intensity)/INTENSITY_SCALE;
    if(red.cycle_iteration<(PWM_PERIOD/INTERRUPT_PERIOD-cycle_intensity))
        RED_PORT=LED_OFF;
    else 
        RED_PORT=LED_ON;
    red.cycle_iteration++;
    if(red.cycle_iteration==PWM_PERIOD/INTERRUPT_PERIOD) red.cycle_iteration=0;          
}
void ledcontroller_pwm_handler_green(void){
    char cycle_intensity=(green.duty_cycle*led_intensity)/INTENSITY_SCALE;
    if(green.cycle_iteration<(PWM_PERIOD/INTERRUPT_PERIOD-cycle_intensity))
        GREEN_PORT=LED_OFF;
    else 
        GREEN_PORT=LED_ON;
    green.cycle_iteration++;
    if(green.cycle_iteration==PWM_PERIOD/INTERRUPT_PERIOD) green.cycle_iteration=0;          
}
void ledcontroller_pwm_handler_blue(void){
    char cycle_intensity=(blue.duty_cycle*led_intensity)/INTENSITY_SCALE;
    if(blue.cycle_iteration<(PWM_PERIOD/INTERRUPT_PERIOD-cycle_intensity))
        BLUE_PORT=LED_OFF;
    else 
        BLUE_PORT=LED_ON;
    blue.cycle_iteration++;
    if(blue.cycle_iteration==PWM_PERIOD/INTERRUPT_PERIOD) blue.cycle_iteration=0;          
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