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
;Archivo cabecera para declaración de Queues y 
;configuraciones de FreeRTOS así como variables y
;constantes comunes de todos los task del sistema 
====================================================*/
#ifndef COLAS_CONFIG_H
#define COLAS_CONFIG_H

#include "EKIA_CONFIG.h"    
#include "FreeRTOS.h" 
#include "semphr.h"    
#include "queue.h"
#include "task.h"
#include "cypins.h"    
#include "FS.h"  
#include "tipos.h"

/*=====================================================
;Declaración de controladores de Queues
====================================================*/  
xQueueHandle sistemaStatus;
xQueueHandle bufferHMI;
xQueueHandle datosGSM;
xQueueHandle cUSB; //Queue para envio de comandos desde puerto USB
xQueueHandle funcion;
xQueueHandle funcionI2C;
xQueueHandle comandosHMI;
xQueueHandle cadena;  
xQueueHandle cadena2;

xQueueHandle configuracionesMaquina;
xQueueHandle configuracionesRemotas;
xQueueHandle configuracionesBandejas;
xQueueHandle configuracionesAccesorios;
xQueueHandle configuracionesTemperatura;
    
xQueueHandle estadoRTOS;
xQueueHandle estadoMemoriaSD;
xQueueHandle estadoConfiguraciones;

xQueueHandle respuestaPantalla;
xQueueHandle notificaciones;
xQueueHandle actualizarInformacion;
xQueueHandle actualizarRegistro;
xQueueHandle enviarRegistro;

xQueueHandle salidaTelemetria;
xQueueHandle entradaTelemetria;
xQueueHandle datosDescargados;   
    
xQueueHandle autorizaciones;  
xQueueHandle autorizacionProveedor;  
    
QueueHandle_t debugMessageQ; 
xQueueHandle comandosSIM; 
    
#define DEBUG(constString/*, statusCode*/) (SendToDebugPrintTask(         \
													(char*)constString/*,        \
													(int)statusCode)*/))
/* Inline function that sends messages to the debug Queue */
void inline static SendToDebugPrintTask(char* stringPtr/*, int statusCode*/)
{
	debug_print_data_t printData = {.stringPointer = stringPtr};
	xQueueSend(debugMessageQ, &printData,1u);
}    

void inline static SendToLogPrintTask(char* stringPtr, int statusCode)
{
	debug_print_data_t printData = {.stringPointer = stringPtr, .code = statusCode};
	xQueueSend(debugMessageQ, &printData,1u);
} 



void colas_Init();
    
#endif
/* [] FIN DE ARCHIVO */
