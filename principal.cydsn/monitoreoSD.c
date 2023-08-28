 /* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * MAQUINA VENDING 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;PROGRAMA ENCARGADO DE INICIALIZAR Y COMANDAR ESCRITURA
;PENDIENTE EN LA MEMORIA SD
====================================================*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "colas.h"
#include "tipos.h"
#include "semaforos.h"

#include "Clock_lento.h"
#include "LED_DEBUG.h"
#include "SDIn.h"
#include "memoriaExt.h"
#include "memEEPROM.h"
#include "monitoreoSD.h"


static void monitorSD(void* pvParameters){ 
    (void)pvParameters;
    uint8 estatusMemoriaSD;
    xContabilidad contabilidad;
    xEventos evento;
    xinfoDescargada descarga;
    habilitarSD();
    for(;;){
        xQueuePeek(estadoMemoriaSD, &estatusMemoriaSD, 10);
        if(SDIn_Read()){//hay una memoria insertada?
            if(xSemaphoreTake(memoriaOcupada,( TickType_t )1)){            
                habilitarSD();
                estatusMemoriaSD = LISTO;
                vTaskDelay(pdMS_TO_TICKS(10));
                xQueueReceive(actualizarInformacion,&contabilidad,10);
                if(contabilidad.operacion==ACTUALIZAR_CONTABILIDAD){
                    Clock_lento_SetDividerRegister(freq_10_Hz,0);
                    escribirContabilidadSD(contabilidad.producto);
                }
                else{
                    Clock_lento_SetDividerRegister(freq_100_Hz,0);
                }
                contabilidad.operacion = 0;
                vTaskDelay(pdMS_TO_TICKS(10));
                xQueueReceive(actualizarRegistro,(void*)&evento,10); 
                if(evento.operacion==ACTUALIZAR_REGISTRO){
                    Clock_lento_SetDividerRegister(freq_10_Hz,0);
                    xQueueSendToBack(enviarRegistro,(void*)&evento,10);
                    escribirRegistroSD(evento);
                }
                else{
                    Clock_lento_SetDividerRegister(freq_100_Hz,0);
                }
                evento.operacion = 0;
                vTaskDelay(pdMS_TO_TICKS(10));
                xQueuePeek(datosDescargados,(void*)&descarga,10); 
                if(descarga.operacion==GUARDAR_DESCARGA){
                    Clock_lento_SetDividerRegister(freq_10_Hz,0);
                    guardarArchivoDescargaSD();
                }
                else{
                    Clock_lento_SetDividerRegister(freq_100_Hz,0);
                }
                descarga.operacion = 0;
                xSemaphoreGive(memoriaOcupada);
            }
            else{
                Clock_lento_SetDividerRegister(freq_10_Hz,0);
            }
            
        }
        else{
            deshabilitarSD();
            estatusMemoriaSD = NO_LISTO;
        }
        xQueueOverwrite(estadoMemoriaSD, (void*)&estatusMemoriaSD);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*******************************************************************************
* DECLARACIÓN DEL TASK DE FUNCIONES DE MONITOREO DE MEMORIA SD
********************************************************************************
*
* Tarea que realiza:
*  Crea el task para control de escritura y monitoreo de la memoria SD
*
*******************************************************************************/
void monitoreoSD_Init(void){ 
    //Creación de un nuevo task
    if(xTaskCreate(monitorSD, //puntero de la función que crea la tarea (nombre de la tarea)
                   "tarea SD", //nombre textual de la tarea, solo útil para depuración
                   memMonitoreoSD, //tamaño de la tarea
                   (void*)NULL, //no se usan parámetros de esta tarea
                   priorMonitoreoSD, //prioridad de la tarea
                   (xTaskHandle*)NULL )!= pdPASS) //no se utilizará manipulador de tarea
    {
        for(;;){} //si llega aquí posiblemente hubo una falta de memoria
    }
}

/* [] END OF FILE */
