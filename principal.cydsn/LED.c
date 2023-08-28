/* ===================================================
 *
 * Copyright EKIA Technology SAS, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * AMIGO 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;PROGRAMA ENCARGADO DE PRENDER Y APAGAR UN LED SUAVEMENTE
;CON EL FIN DE VERIFICAR EL FUNCIONAMIENTO DEL FreeRTOS
;MODIFICANDO EL CICLO ÚTIL DE UN PWM
====================================================*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"
#include "PWM_Led.h"
#include "colas.h"
#include "tipos.h"
#include "semaforos.h"


/*******************************************************************************
* TASK PRINCIPAL DEL DIMMER DEL LED DE ESTADO
********************************************************************************
*
* Tarea que realiza:
*  Esta tarea se encarga de controlar el brillo de un LED de estado que indicará
*  que se encuentra funcionando correctamente todo el programa en el psoc.
*
*******************************************************************************/
static void Led_Dimmer(void* pvParameters){ 
    (void)pvParameters;
    
    //inicialización de componentes y variables
    _Bool incrementar = 1; //la bandera incrementar indica si incrementa o decrementa el ciclo util
    uint16 duty;
    PWM_Led_Start();        //se inicializa el PWM

    int factor = PWM_Led_ReadPeriod(); //se calculará un factor basado en el periodo del PWM
    if (factor >=100){       //este factor sera usado para escalar el incremento en el ciclo util
        factor = factor/100; //de modo que se logren variaciones de 1% cada 10ms
    }
    else{
        factor = 1;
    }
    
    //proceso principal
    for(;;){
        duty = PWM_Led_ReadCompare(); //se lee el valor actual del ciclo util
        if (incrementar==1){          //si la bandera incrementar está en 1, significa que debe aumentar el DT.
            if(duty<(100*factor)){
                PWM_Led_WriteCompare(duty+factor);
            }
            else{
                PWM_Led_WriteCompare(100*factor);
                incrementar = 0;
            }
        }
        else{ //si la bandera se encuentra en 0, decrementará el DT
            if(duty>factor){
                PWM_Led_WriteCompare(duty-factor);
            }
            else{
                PWM_Led_WriteCompare(factor);
                incrementar = 1;
            }
        }
        //vTaskDelay(200/portTICK_PERIOD_100US); //temporizacion de 20ms reales (gracias al FreeRTOS)
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/*******************************************************************************
* DECLARACIÓN DEL TASK DE DIMMER PARA EL LED
********************************************************************************
*
* Tarea que realiza:
*  Crea el task para encendido y apagado del LED de estado del programa 
*  con una prioridad de 5 y un tamaño reservado de 100 bytes, no tiene manejador asignado
*
*******************************************************************************/
void LED_Init(void){
    //Creación de un nuevo task
    if(xTaskCreate(Led_Dimmer, //puntero de la función que crea la tarea (nombre de la tarea)
                   "Led Blink", //nombre textual de la tarea, solo útil para depuración
                   memLED, //tamaño de la tarea
                   (void*)NULL, //no se usan parámetros de esta tarea
                   priorLED, //prioridad de la tarea
                   (xTaskHandle*)NULL )!= pdPASS) //no se utilizará manipulador de tarea
    {
        for(;;){} //si llega aquí posiblemente hubo una falta de memoria
    }
}

/* [] FIN DE ARCHIVO */
