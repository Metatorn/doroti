 /* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2020
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * DOROTI 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;PROGRAMA ENCARGADO DE CONTROL DE VARIOS SISTEMAS
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

#include "controles.h"
#include "EKIA_CONFIG.h"
#include "math.h"

#include "LED.h"
#include "Motores.h"
#include "PWM_Motores.h"
#include "Bloqueo_Fila.h"
#include "Bloqueo_Columna.h"
#include "Refrigeracion.h"

CYBIT enfriando = pdFALSE;

/*******************************************************************************
* FUNCION DE ACTIVACIÓN DE MOTORES
********************************************************************************
* Función que activa los motores, activando y desactivando la fila o columna 
* correspondientes, como entrada recibe el número del motor que quiera moverse
* tener en cuenta que no es el mismo número de producto
*
* Retorna: bandera booleana que indica si se dispensó correctamente un producto
*       movimientoMotorOk = TRUE si se detectó la caida de un producto
*       movimientoMotorOk = FALSE si no se detectó caida y se superó el tiempo
*******************************************************************************/

uint8 activarMotor(uint8 motor){
    uint8 producto_Entregado=pdFALSE;
    uint8 bandeja=0;
    uint8 columnaMotor=0;
    uint16 duty = 1;
    uint8 intentos = 0;
    uint8 indice = 0;
    uint8 contador = 0;
    CYBIT puertaAbierta1=pdFALSE, puertaAbierta2=pdFALSE;
    int tiempo=0;
    int cicloMax = 0;
    xMedidas lecturas;
    xConfiguraciones parametrosMaquina;
    //Control_Reg_Write(Control_Reg_Read()|0x04); //bloquee la compuerta de entrega para evitar errores y posibles robos  
    bandeja = ((motor-1)/NUMERO_PRODUCTOS_BANDEJA)+1;
    columnaMotor = ((motor-1)%NUMERO_PRODUCTOS_BANDEJA)+1;
    columnaMotor = columnaMotor<<3;     //active el ULN correspondiente para energizar la columna
    xQueuePeek(parametros,(void*)&parametrosMaquina,100);
    cicloMax = (4095*parametrosMaquina.cicloUtil)/100;
    xQueueOverwrite(sensorPuerta1,(void*)&puertaAbierta1);
    xQueueOverwrite(sensorPuerta2,(void*)&puertaAbierta2);
    while((!producto_Entregado)&&(intentos<NUM_INTENTOS)){
        Motores_Write(bandeja|columnaMotor);
        for(duty=10;duty<cicloMax;duty++){
            PWM_Motores_WriteCompare(duty);
            CyDelayUs(TIEMPO_ARRANQUE_SUAVE);
        }
        //movimiento inicial para entrar switch
        CyDelay(parametrosMaquina.tiempoBandeja[bandeja-1]);
        //CyDelay(T_MOVER_MOTOR);
        //movimiento con switch(sin energizar pin de activación sin switch)
        Motores_Write(bandeja & 0x07);
        while(tiempo<parametrosMaquina.tiempoMovimientoMax){
            CyDelayUs(500);
            tiempo++;
        }
        tiempo=0;
        vTaskDelay(pdMS_TO_TICKS(500));
        producto_Entregado=pdFALSE;
        if(xSemaphoreTake(pruebaDesactivada,100)){
            xSemaphoreGive(pruebaDesactivada);
            while(tiempo<T_ESPERA_CAIDA){
                contador=0;
                for(indice=0;indice<50;indice++){
                    xQueuePeek(sensorPuerta1,(void*)&puertaAbierta1,1);
                    if(puertaAbierta1||puertaAbierta2){
                        producto_Entregado=2; //el movimiento finalizó por apertura de puerta
                        tiempo = T_ESPERA_CAIDA;
                        indice = 50;
                    }
                    xQueuePeek(medidas,(void*)&lecturas,100);
                    if(lecturas.peso>=parametrosMaquina.pesoMinimo){
                        contador++;
                        indice = 0;
                    }
                    CyDelay(1);
                    if(contador>=40){
                        indice = 50;
                        producto_Entregado=pdTRUE; //el movimiento finalizó correctamente 
                        tiempo = T_ESPERA_CAIDA;
                    }
                }
                
                CyDelay(10);
                tiempo++;
            }
        }
        tiempo=0;
        intentos++;
    }
    Motores_Write(pdFALSE);
    //Control_Reg_Write(Control_Reg_Read()&0xFB);  //desbloquea la canasta para permitir sacar productos
    puertaAbierta1 = pdFALSE;
    puertaAbierta2 = pdFALSE;
    xQueueOverwrite(sensorPuerta1,(void*)&puertaAbierta1);
    xQueueOverwrite(sensorPuerta2,(void*)&puertaAbierta2);
    return producto_Entregado;
}

/*******************************************************************************
* FUNCION DE APERTURA DE SLOTS
********************************************************************************
* Función que activa los bloqueos de los slots
*******************************************************************************/

void abrirSlot(uint8 slot){
    uint8 bandeja = ~(0xFF & (int)(pow(2,((slot-1)/NUMERO_PRODUCTOS_BANDEJA))));
    uint8 columna = ~(0xFF & (int)(pow(2,((slot-1)%NUMERO_PRODUCTOS_BANDEJA))));
    Bloqueo_Columna_Write(columna);
    Bloqueo_Fila_Write(bandeja);
    vTaskDelay(pdMS_TO_TICKS(1500));
    Bloqueo_Columna_Write(0xFF);
    Bloqueo_Fila_Write(0xFF);
}

/*******************************************************************************
* CONTROL DE TEMPERATURA
********************************************************************************
* Función que controla el sistema de refrigeracion
*******************************************************************************/

void control_temperatura(){
    xMedidas mediciones;
    xConfiguracionTemperatura configuracionesTemp;
    xQueuePeek(configuracionesTemperatura,(void*)&configuracionesTemp,10);
    xQueuePeek(medidas,(void*)&mediciones,100);
    
    if(configuracionesTemp.activado==pdTRUE){ //verifica si el control se encuentra activo
        if(enfriando){ //verifica si la maquina se encuentra enfriando
            if(mediciones.ambienteProm.temperatura<=(configuracionesTemp.gradosC-configuracionesTemp.HisteresisInfer)){
                Refrigeracion_Write(pdFALSE);
                enfriando = pdFALSE;
            }
            else{
                Refrigeracion_Write(pdTRUE);
            }
        }
        else{
            if(mediciones.ambienteProm.temperatura>=(configuracionesTemp.gradosC+configuracionesTemp.HisteresisSuper)){
                Refrigeracion_Write(pdTRUE);
                enfriando = pdTRUE;
            }
            else{
                Refrigeracion_Write(pdFALSE);
            }
        }
    }
    else{
        Refrigeracion_Write(pdFALSE);
    }
}



static void control(void* pvParameters){ 
    (void)pvParameters;
    PWM_Motores_Start();
    int motor = 0, slot = 0;
    CYBIT canasta = 0;
    for(;;){
        if(xQueueReceive(solicitudDispensar,(void*)&motor,100)){
            xQueueOverwrite(sensorPuerta1,(void*)&canasta);
            xQueueOverwrite(sensorPuerta2,(void*)&canasta);
            uint8 solicitud = SET_CERO_PESO;
            xQueueSend(solicitudPeso,(void*)&solicitud,100);
            CyDelay(200);
            uint8 respuesta = activarMotor(motor);
            xQueueOverwrite(respuestaDispensar,(void*)&respuesta);
        }
        if(xQueueReceive(solicitudApertura,(void*)&slot,100)){
            abrirSlot(slot);
        }
        control_temperatura();
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
void control_Init(void){ 
    //Creación de un nuevo task
    if(xTaskCreate(control, //puntero de la función que crea la tarea (nombre de la tarea)
                   "tarea de monitoreo", //nombre textual de la tarea, solo útil para depuración
                   memControl, //tamaño de la tarea
                   (void*)NULL, //no se usan parámetros de esta tarea
                   priorControl, //prioridad de la tarea
                   (xTaskHandle*)NULL )!= pdPASS) //no se utilizará manipulador de tarea
    {
        for(;;){} //si llega aquí posiblemente hubo una falta de memoria
    }
}

/* [] END OF FILE */
