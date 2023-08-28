 /* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * CAFETERA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;PROGRAMA ENCARGADO DE MONITOREAR SISTEMAS
====================================================*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cyapicallbacks.h"

#include "tipos.h"
#include "colas.h"
#include "semaforos.h"
#include "monitor.h"

#include "EKIA_CONFIG.h"
#include "math.h"
//#include "LED.h"

#include "LED.h"
#include "hx711.h"
#include "PWM_LED.h"
#include "Sensor_Canasta1.h"
#include "Sensor_Canasta2.h"
#include "isr_Sensor1.h"
#include "isr_Sensor2.h"

int contadorSensor1 = 0;
int contadorSensor2 = 0;

CY_ISR(sensorCanastaAbierta1){
    CyDelayUs(2);
    CYBIT puertaAbierta1 = pdFALSE;
    xQueuePeekFromISR(sensorPuerta1,(void*)&puertaAbierta1);
    if(Sensor_Canasta1_Read()){
        contadorSensor1++;
    }
    else{
        contadorSensor1 = 0;
    }
    if (contadorSensor1 >= 1){ //Normal 10
        contadorSensor1 = 0;
        puertaAbierta1 = pdTRUE; //cambiar por cola de comunicacion
        xQueueOverwriteFromISR(sensorPuerta1,(void*)&puertaAbierta1,NULL);
    }
    Sensor_Canasta1_ClearInterrupt();
}

CY_ISR(sensorCanastaAbierta2){
    CyDelayUs(2);
    CYBIT puertaAbierta2 = pdFALSE;
    xQueuePeekFromISR(sensorPuerta2,(void*)&puertaAbierta2);
    if(Sensor_Canasta2_Read()){
        contadorSensor2++;
    }
    else{
        contadorSensor2 = 0;
    }
    if (contadorSensor2 >= 1){ //Normal 5
        contadorSensor2 = 0;
        puertaAbierta2 = pdTRUE;
        xQueueOverwriteFromISR(sensorPuerta2,(void*)&puertaAbierta2,NULL);
    }
    Sensor_Canasta2_ClearInterrupt();
}

void cargarConfiguraciones(){
    int i = 0;
    xConfiguraciones parametrosMaquina;
    xConfiguracionTemperatura parametrosTemp;
    xQueuePeek(parametros,(void*)&parametrosMaquina,100);
    parametrosMaquina.pesoMinimo = PESO_MINIMO;
    parametrosMaquina.escala = ESCALA;
    parametrosMaquina.tiempoMovimientoMax = T_MOVIMIENTO;
    parametrosMaquina.cicloUtil = 100;
    for(i=0;i<NUMERO_BANDEJAS;i++){
        parametrosMaquina.tiempoBandeja[i]=T_MOVER_MOTOR;
    }
    xQueuePeek(configuracionesTemperatura,(void*)&parametrosTemp,100);
    parametrosTemp.activado = pdFALSE;
    parametrosTemp.gradosC = TEMPERATURA_DEFAULT;
    parametrosTemp.HisteresisInfer = HISTERESIS_INFERIOR;
    parametrosTemp.HisteresisSuper = HISTERESIS_SUPERIOR;
    xQueueOverwrite(parametros,(void*)&parametrosMaquina);
}

static void monitoreo(void* pvParameters){ 
    (void)pvParameters;
    PWM_LED_Start();
    cargarConfiguraciones();
    xConfiguraciones parametrosMaquina;
    isr_Sensor1_StartEx(sensorCanastaAbierta1);
    #if(NUMERO_SENSORES==2)
        isr_Sensor2_StartEx(sensorCanastaAbierta2);
    #endif
    for(;;)
    {
        xQueuePeek(parametros,(void*)&parametrosMaquina,100);
        if(xSemaphoreTake(pesoOcupado,100)){
            definirEscalaPeso(parametrosMaquina.escala);
            definirPesoMinimo(parametrosMaquina.pesoMinimo);
            xSemaphoreGive(pesoOcupado);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/*******************************************************************************
* DECLARACIÓN DEL TASK DE FUNCIONES GENERALES
********************************************************************************
*
* Tarea que realiza:
*  Crea el task para control de perifericos generales
*
*******************************************************************************/
void monitoreo_Init(void){ 
    //Creación de un nuevo task
    if(xTaskCreate(monitoreo, //puntero de la función que crea la tarea (nombre de la tarea)
                   "tarea de monitoreo", //nombre textual de la tarea, solo útil para depuración
                   memMonitor, //tamaño de la tarea
                   (void*)NULL, //no se usan parámetros de esta tarea
                   priorMonitor, //prioridad de la tarea
                   (xTaskHandle*)NULL )!= pdPASS) //no se utilizará manipulador de tarea
    {
        for(;;){} //si llega aquí posiblemente hubo una falta de memoria
    }
}

/* [] FIN DE ARCHIVO */
