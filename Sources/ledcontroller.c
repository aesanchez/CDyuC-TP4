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

char sweep_on; unsigned char sweep_counter;

void ledcontroller_init(){
    red.cycle_iteration=0;
    green.cycle_iteration=0;
    blue.cycle_iteration=0;

    blink_on=0;
    sweep_on=0;
    system_state=SYSTEM_ON;
    led_intensity=1.0; 
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