#include "shell.h"
#include "keyevent.h"

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


void shell_init(void){
    //TODO INIT
}

void shell_run(){
    if(keyevent_is_empty())return;
    key=keyevent_pop;
    for(key_aux=0;key_aux<NUMBER_OF_COMMANDS;key_aux++){
        if(key==key_association[key_aux]) break;
    }
    if(key_aux==NUMBER_OF_COMMANDS)return;//ERROR
    (*COMMANDS_FUNC[key_aux])();
}















