# Circuitos Digitales y Microcontroladores
Repsitorio para el desarrollo del cuarto Trabajo Practico de la materia Circuitos Digitales y Microcontroladores de la [UNLP](http://www.unlp.edu.ar/)
## Objetivo
Implementar un controlador de led RGB mediante un teclado matricial y un potenciometro. El mismo debera cumplir los siguientes funcionalidades:
* Permitir el control del sistema desde el teclado matricial.
* Poseer un set de comandos para realizar las diferentes acciones.
* Generacion de señales PWM para controlar la intensidad de los leds.
* Interpretar el accionar del potenciometro mediante un conversor analogico-digital.
* En base estas lecturas del potenciometro, influenciar en la intensidad total del sistema.
## Bonus
Se implemento un algoritmo para que el led haga un barrido de colores en forma de "arcoiris". Efecto que se puede apreciar en esta imagen:

![Rainbow](https://github.com/trorik23/CDyuC-TP4/blob/master/rainbow_example.png)
## Microcontrolador
Para la resolucion, la catedra nos proporciona un microcontrolador de la familia HC9S08 (SH8), en particular el CPJ junto con un modulo EBDM. Un teclado matricial 4x4, un potenciometro y un led RGB.
## Software
CodeWarriors v10.1
