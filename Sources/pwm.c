#define LOW 0 
#define HIGH 1 

const char period=10;

void pwmgenerator(void) {
  char PWM_OUTPUT_VALUE=LOW;
  char duty_cycle=3;//from 1 to 10
  char signal=0;
  while(true){
    //sleep 1 iteration
    if(signal<(period-duty_cycle)) PWM_OUTPUT_VALUE=LOW;
    else PWM_OUTPUT_VALUE=HIGH;
    printf("%c",PWM_OUTPUT_VALUE);
    signal++;
    if(signal==period) signal=0;
    i++;
  }
}
