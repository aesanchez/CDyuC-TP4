#include <mc9s08sh8.h>
#include "ledcontroller.h"
#include "keyevent.h"

// todos los periodos definidos en ms

// INTERRUPT_PERIOD indica cada cuanto se llama ledcontroller_interrupt_handler
// Todos lo periodos indicados para cada funcionalidad del sistema, deberian ser
// multiplos es INTERRUPT_PERIOD
#define INTERRUPT_PERIOD 1

#define PWM_PERIOD 10 // periodo de la señal PWM

// MAX_DUTY_CYCLE indica el valor correspondiente a ciclo de trabajo 100%
#define MAX_DUTY_CYCLE (PWM_PERIOD/INTERRUPT_PERIOD)
#define MIN_DUTY_CYCLE 0 

#define SWEEP_COLOR_PERIOD 50 // tiempo activo de cada color en el barrido

#define BLINK_MAX 1000 // periodo maximo de blinkeo 
#define BLINK_MIN 100 // periodo minimo de blinkeo
#define BLINK_STEP 300 // valor de salto al modificar el periodo del blinkeo

// valores correspondientes a cada color dentro del arreglo de leds rgb[]
#define RED 0
#define GREEN 1
#define BLUE 2

void state_on(void);
void state_off(void);
void set_white(void);
void blink_vel_down(void);
void blink_vel_up(void);
void blink_toggle(void);
void sweep_toggle(void);
void led_activate(char);
void led_desactivate(char);
void led_toggle_state(char);
void led_on(char);
void led_off(char);
void led_intensity_down(char);
void led_intensity_up(char);
void ledcontroller_pwm_handler(char);
void ledcontroller_blink_handler(void);
void ledcontroller_sweep_handler(void);

// current_state indica el modo en que se encuentra el ledcontroller
// OFF: sistema completamente apagado
// NORMAL: controlar manual del led, acorde a las teclas presionadas
// BLINKING: titileo del led a una frecuencia determinada
// SWEEPING: barrido de colores prefijado
typedef enum{OFF,NORMAL,BLINKING,SWEEPING} state;
state current_state;

// estructura para el almacenamiento de informacion necesaria de cada led
struct led_type{
    char duty_cycle; // indica el ciclo de trabajo actual. Permite controlar la intensidad individual del led
    char cycle_iteration; // iteracion actual para el control de PWM por software
    char state; // indica si el led esta apagado o prendido. state=1 >> prendido 
    char mask; // mascara para permitir el control de los puertos correspondientes a cada led. Permite abstraer la direccion fisica del puerto
};

// rgb es un arreglo para contener la informacion de los 3 leds Red, Green y Blue
struct led_type rgb[3];

// led_intensity indica la intensidad total del sistema. Establece el % maximo de intensidad en
// la que el ledcontroller puede trabajar. Sera el que se vera modificada por el potenciometro
char led_intensity;

// variables correspondientes para el control de blinkeo
int blink_period = 700;
unsigned int blink_interrupt_counter;
char unsigned blink_state;//1->ON

// variables correspondientes para el control de barrido de colores
unsigned int sweep_interrupt_counter;
char sweep_color_counter;

// variable auxiliar para la lectura de eventos de tecla
static char key;

// ledcontroller_run es el encargado de leer los eventos de teclas presionadas.
// En base a la tecla presionada y si el ledcontroller esta apagado o no, deriva 
// la accion a sus funcion interna correspondiente
void ledcontroller_run(){
    if(keyevent_is_empty())return;
    key=keyevent_pop();
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
    if(current_state==OFF){
        // en el modo OFF, se ignoran todas las teclas menos la
        // correspondiente a la tecla de encender el ledcontroller
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
            case '1':
            case '2':
            case '3': led_toggle_state(((key-'0')-1) % 3); break;
            case '4':
            case '5':
            case '6': led_intensity_up(((key-'0')-1) % 3); break;
            case '7':
            case '8':
            case '9': led_intensity_down(((key-'0')-1) % 3);break;
            default: /*error*/ break;
        }
    }
}

// state_on cambia el estado actual al estado NORMAL. Al intercambiarse entre los modos prendidos
// y apagado del sistema, la configuracion independiente de cada led se conserva, no se resetea.
void state_on(void){
    current_state=NORMAL;
}

// state_off apaga completamente el sistema, podiendo volver a prenderse solo con la tecla
// correspondiente al encendido. Apaga forzadamente los leds por si estaban a la mitad del
// control de PWM. No desactiva ni desconfigura nada, permitiendo que al reanudarse se mantengan
// los estados en que estaba antes cada led.
void state_off(void){
    current_state=OFF;
    led_off(RED);
    led_off(BLUE);
    led_off(GREEN);
}

// set_white setea el led RGB al color blanco. Fuerza el estado NORMAL, lo que significa que desactiva
// el barrido o el titileo si es que se encontraban activos. Para prender en blanco, se activan y se
// setea la intensidad de cada led al maximo.
void set_white(void){
    current_state=NORMAL;
    rgb[RED].duty_cycle=MAX_DUTY_CYCLE;
    rgb[GREEN].duty_cycle=MAX_DUTY_CYCLE;
    rgb[BLUE].duty_cycle=MAX_DUTY_CYCLE;
    led_activate(RED);
    led_activate(GREEN);
    led_activate(BLUE);
}

// ledcontroller_init se encarga de las configuraciones iniciales de ledcontroller. Inicializa
// los puertos en que se conecta cada led (PTC pines 1,2 y 3) e inicializa todas las variables
// internas necesarias. Se empieza en el estado apagado.
void ledcontroller_init(void){
    current_state=OFF;    
    // configurar los puertos como salida
    PTCDD_PTCDD1=1;
    PTCDD_PTCDD2=1;
    PTCDD_PTCDD3=1;
    // modo high drive
    PTCDS=0x0F;
    // setear las mascaras de cada led al bit que corresponden en PTC.
    // Esto nos permite abstraer la direccion fisica de cada led y utilizar una sola
    // funcion para el prendido y apagado de los leds
    rgb[RED].mask=PTCD_PTCD1_MASK;
    rgb[GREEN].mask=PTCD_PTCD2_MASK;
    rgb[BLUE].mask=PTCD_PTCD3_MASK;
    // arrancan todos los leds en su maxima intensidad
    rgb[RED].duty_cycle=MAX_DUTY_CYCLE;
    rgb[GREEN].duty_cycle=MAX_DUTY_CYCLE;
    rgb[BLUE].duty_cycle=MAX_DUTY_CYCLE;
    // se activan los los leds
    rgb[RED].state=1;
    rgb[GREEN].state=1;
    rgb[BLUE].state=1;
    // se fuerzan las salidas de los leds a apagado, por si se encontraban activas
    led_off(RED);
    led_off(GREEN);
    led_off(BLUE);
}

// led_desactivate desactiva y fuerza el apagado del led indicado. Recibe como parametro el
// indice del led a apagar dentro del arreglo rgb.
void led_desactivate(char led_index){
    rgb[led_index].state=0;
    led_off(led_index);    
}

// led_activate activa el led indicado. Recibe como parametro el indice del led 
// a prender dentro del arreglo rgb. No hace falta forzae a "prendido" porque el mismo
// pwm lo va a prender
void led_activate(char led_index){
    rgb[led_index].state=1;
}

// led_toggle_state activa o desactiva el estado del led indicado. Recibe como parametro el
// indice dentro del arreglo rgb. Realiza el cambio de estado con las funciones led_activate
// y led_desactivate segun su estado actual
void led_toggle_state(char led_index){
    if(rgb[led_index].state) led_desactivate(led_index);
    else led_activate(led_index);
}

// led_on prende el led indicado modificando la salida correspondiente del puerto PTC.
// Recibe como parametro el indice dentro del arreglo rgb.
// Esto lo logra de forma general, utilizando la mascara de cada led. Dado las caracteristicas
// del led RGB, el led se prender con el valor 0 
void led_on(char led_index){
    PTCD= PTCD & (~rgb[led_index].mask);
    /*
    PTCD  xxxxxxxx
    and
    !mask 11111101
    --------------
    resul xxxxxx0x
    */    
}

// led_off apaga el led indicado de forma analoga a led_on. Led apagado se da con el valor HIGH
void led_off(char led_index){
    PTCD = PTCD | rgb[led_index].mask;
    /*
    PTCD  xxxxxxxx
    or
    mask  00000010
    --------------
    resul xxxxxx1x
    */
}

// led_intensity_up aumenta la intensidad del led indicado por parametro. Aumentar la intensidad
// significa aumentarle el ciclo de trabajo de la señal PWM. Se limita a MAX_DUTY_CYCLE.
// En particular te deja cambiar la intensidad ya sea en modo NORMAL y BLINKING, pero no en
// SWEEPING dado que generaria inconsistencias en el barrido de colores.
void led_intensity_up(char led_index){
    if(current_state==SWEEPING) return;
    if(rgb[led_index].duty_cycle==MAX_DUTY_CYCLE) return;
    rgb[led_index].duty_cycle++;
}

// led_intensity_down disminuye la intensidad del led indicado por parametro. Se resuelve de forma
// analoga a led_intensity_down
void led_intensity_down(char led_index){
    if(current_state==SWEEPING) return;
    if(rgb[led_index].duty_cycle==MIN_DUTY_CYCLE) return;
    rgb[led_index].duty_cycle--;
}

// ledcontroller_interrupt_handler funcion que es llamada cada INTERRUPT_PERIOD, osea 1 ms en el caso normal.
// Se encarga de los llamados a las funciones del control del PWM de cada led, del barrido y del blinkeo,
// segun corresponda.
void ledcontroller_interrupt_handler(void){
    if(current_state==SWEEPING)ledcontroller_sweep_handler();
    if(current_state==BLINKING)ledcontroller_blink_handler();
    // No atualiza, ni prende, los leds si es que se encuentra en modo titileo
    // y en el estado de titileo apagado
    if(current_state==BLINKING && blink_state==0) return;
    if(rgb[RED].state)ledcontroller_pwm_handler(RED);
    if(rgb[GREEN].state)ledcontroller_pwm_handler(GREEN);
    if(rgb[BLUE].state)ledcontroller_pwm_handler(BLUE);    
    
}

// ledcontroller_pwm_handler se encarga de la generacion de la señal PWM por software, del led indicado por paramtro.
// Permite controlar la intensidad del led de acuerdo a su ciclo de trabajo y a la maxima intensidad del
// led RGB en general, dada por led_intensity. 
// Utiliza cycle_iteration para controlar, en cada iteracion dentro de un mismo perido de señal PWM,
// el apagado o encendido del led segun corresponda.
void ledcontroller_pwm_handler(char led_index){
    /* ejemplo de funcionamiento
        Asumiendo duty_cycle: 8(80%) y led_intensity: 5(50%)
        Por lo que true_intensity: 40%
        Iteracion: 01234567890123456789
        Señal:     ____------____------
        Notar que el ciclo de trabajo indica el % en que la señal esta activa,
        y en nuestro caso estar activa significa estar en 0
    */
    char true_intensity=(rgb[led_index].duty_cycle*led_intensity)/INTENSITY_SCALE;
    if(rgb[led_index].cycle_iteration<(PWM_PERIOD/INTERRUPT_PERIOD-true_intensity))
        led_off(led_index);
    else 
        led_on(led_index);
    rgb[led_index].cycle_iteration++;
    if(rgb[led_index].cycle_iteration==PWM_PERIOD/INTERRUPT_PERIOD) rgb[led_index].cycle_iteration=0;          
}

// ledcontroller_set_intensity establece la intensidad maxima recibida por parametro. El parametro
// debe ser coherente a la escala de la intensidad dada por INTENSITY_SCALE
void ledcontroller_set_intensity(char intensity){
    led_intensity=intensity;
}

// ledcontroller_blink_handler se encarga de intercambiar los estados de titileo con el periodo
// indicado por blink_period. Dicho periodo variara segun las indicaciones del usuario. Para lograr
// el titieo se intercambia entre blink_state=0 (apagado) y blink_state=1(prendido). Al apagar los leds
// no se desactivan sino que simplemente se fuerza la salida del led a apgado, y no se prenderan dado que
// ledcontroller_interrupt_handler no atendera la generacion de la señal PWM si se encuentra en este
// estado de titileo. Esto genera que el usuario pueda activar y desactivar cada led independientemente,
// sin importar que se encuentre en este modo. 
void ledcontroller_blink_handler(void){
    if(blink_interrupt_counter++<(blink_period/INTERRUPT_PERIOD))return;
    blink_interrupt_counter=0;
    if(blink_state){
        blink_state=0;
        led_off(RED);
        led_off(GREEN);
        led_off(BLUE);
    }else{
        blink_state=1;
    } 
}

// blink_toggle cambia entre los modos NORMAL y BLINKING segun en que modo se encontraba.
// Si se activa el modo BLINKING, hace las inicializaciones correspondientes.
void blink_toggle(void){
    if(current_state!=BLINKING){
        current_state=BLINKING;
        //init blink
        blink_interrupt_counter=0;
        blink_state=1;
        
    }else{
        current_state=NORMAL;
    }
}

// blink_vel_down disminuye la frecuencia de titileo
void blink_vel_down(void){
    if(blink_period==BLINK_MAX)return;
    blink_period+=BLINK_STEP;
}

// blinkc_vel_up aumenta la frecuencia de titileo
void blink_vel_up(void){
    if(blink_period==BLINK_MIN)return;
    blink_period-=BLINK_STEP;
}

// ledcontroler_is_on devuelve si el estado del ledcontroller esta apagado o prendido. Permite la validacion
// de si se debe llamar ledcontroller_interrupt_handler en caso de que el sistema este prendido.
// 1: prendido(NORMAL,BLINKING o SWEEPING)
// 0: apagado(OFF)
char ledcontroller_is_on(void){
    if(current_state==OFF) return 0;
    else return 1;
}

// sweep_toggle cambia entre los modos NORMAL y SWEEPING segun en que modo se encontraba.
// Si se activa el modo SWEEPING, hace las inicializaciones correspondientes. A diferencia
// del modo BLINKING, este estado si fuerza la activacion de todos los leds, para lograr el
// efecto de arcoiris deseado. De todas maneras, luego de la inicializacion, se permitira al
// usuario activar o desactivar cada led por separado. No se permitira modificar sus intensidades
// individuales.
void sweep_toggle(void){
    if(current_state!=SWEEPING){
        current_state=SWEEPING;
        //init sweep
        sweep_interrupt_counter=0;
        sweep_color_counter=0;
        led_activate(RED);
        led_activate(GREEN);
        led_activate(BLUE);
        rgb[RED].duty_cycle=MAX_DUTY_CYCLE;
        rgb[GREEN].duty_cycle=MIN_DUTY_CYCLE;
        rgb[BLUE].duty_cycle=MIN_DUTY_CYCLE;        
    }else{
        current_state=NORMAL;
    }
}

// ledcontroller_sweep_handler se encarga de barrido de colores en forma de arcoiris, intercalando
// por todas las combinaciones de colores RGB (de forma simplificada). Cada uno de los colores se
// va a mantener un tiempo definido por SWEEP_COLOR_PERIOD.
void ledcontroller_sweep_handler(void){
    if(sweep_interrupt_counter++<(SWEEP_COLOR_PERIOD/INTERRUPT_PERIOD))return;
    sweep_interrupt_counter=0;
    // El algoritmo de barrido divide el barrido en 6 partes, en las que en
    // cada una mantiene uno de los colores con intensidad maxima, uno de los colores con
    // intensidad minima y el ultimo color modificandose.

    //no importa si estan prendidos o no-->barrido
    switch(sweep_color_counter/MAX_DUTY_CYCLE){
        case 0:  rgb[GREEN].duty_cycle++; break;//0-9
        case 1:  rgb[RED].duty_cycle--;   break;//10-19
        case 2:  rgb[BLUE].duty_cycle++;  break;//20-29
        case 3:  rgb[GREEN].duty_cycle--; break;//30-39
        case 4:  rgb[RED].duty_cycle++;   break;//40-49
        case 5:  rgb[BLUE].duty_cycle--;  break;//50-59
    }
    sweep_color_counter=(sweep_color_counter+1) % (MAX_DUTY_CYCLE*6);

    /* ejemplo
        Tomando como ejemplo que los duty_cycles solo pueden ser 0-3
        Iteracion: 0 1 2|3 4 5|6 7 8|9 10 11|12 13 14|15 16 17|0 1 2...
        --------------------------------------------------------------------
        R:         3 3 3|2 1 0|0 0 0|0  0  0| 1  2  3| 3  3  3|3 3 3...          
        G:         1 2 3|3 3 3|3 3 3|2  1  0| 0  0  0| 0  0  0|1 2 3...
        B:         0 0 0|0 0 0|1 2 3|3  3  3| 3  3  3| 1  2  3|0 0 0...
    */
}