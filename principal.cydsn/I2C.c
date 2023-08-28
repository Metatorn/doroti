/* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
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
;Funcoones comunes para el manejo del bus principal
====================================================*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "I2C.h"
#include "colas.h"
#include "tipos.h"
#include "semaforos.h"

#include "BusPrincipal.h"
#include "Bus_SDA.h"
#include "Bus_SCL.h"
//#include "LED_DEBUG.h"

void habilitar_busPrincipal(){
    BusPrincipal_Start();
    Bus_SDA_SetDriveMode(Bus_SDA_DM_RES_UP);
    Bus_SCL_SetDriveMode(Bus_SCL_DM_RES_UP);
}

void deshabilitar_busPrincipal(){
    BusPrincipal_MasterClearReadBuf();
    BusPrincipal_MasterClearWriteBuf();
    BusPrincipal_MasterClearStatus();
    BusPrincipal_Stop();
}

void reiniciar_busPrincipal(){
    deshabilitar_busPrincipal();
    vTaskDelay(pdMS_TO_TICKS(1000));
    BusPrincipal_Init();
    habilitar_busPrincipal();
    BusPrincipal_Init();
}

uint8 verificarConexionEsclavo(uint8 direccion){
    uint8 presencia = pdFALSE;
    if(!BusPrincipal_MasterSendStart(direccion, BusPrincipal_WRITE_XFER_MODE)){
        presencia = pdTRUE;
    }
    BusPrincipal_MasterReadByte(0);
    BusPrincipal_MasterSendStop(); //reset I2C bus
    BusPrincipal_EnableInt();
    return presencia;
}

CYBIT bus_escribir_Comando(uint8 Direccion, uint8* bufferEscritura){
    CYBIT  comando_ok = pdFALSE;
    uint8 estado;
    uint16 contador=0;
    //intenta la comunicación mientras el esclavo responde sin errores
    vTaskDelay(pdMS_TO_TICKS(1));
    
    do{
        estado = BusPrincipal_MasterWriteBuf(
            Direccion, 
            bufferEscritura,
            LARGO_BUFFER, 
            BusPrincipal_MODE_COMPLETE_XFER);
        contador++;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    while ((estado != BusPrincipal_MSTR_NO_ERROR)&&(contador<I2C_TIMEOUT));
    if((contador<I2C_TIMEOUT)){
        contador = 0;
        // Espere a que se complete la transferencia de datos
        do{
            estado = BusPrincipal_MasterStatus();
            contador++;
            vTaskDelay(pdMS_TO_TICKS(1));
        } 
        while((estado & BusPrincipal_MSTAT_XFER_INP)&&(contador<I2C_TIMEOUT));
        estado = BusPrincipal_MasterClearStatus();
        if(estado & 0xF0){
            comando_ok = pdFALSE;
        }
        else{
            comando_ok = pdTRUE; 
        }
    }
    BusPrincipal_MasterClearWriteBuf();
    vTaskDelay(pdMS_TO_TICKS(10));
    return comando_ok;
}

CYBIT bus_leer_comando(uint8 Direccion, uint8* bufferLectura){
    CYBIT comando_ok = pdFALSE;
    uint8 estado;
    uint16 contador=0;
    //intenta la comunicación mientras el esclavo responde sin errores
    vTaskDelay(pdMS_TO_TICKS(20));
    do{
        estado = BusPrincipal_MasterReadBuf(
            Direccion, 
            bufferLectura, 
			LARGO_BUFFER, 
            BusPrincipal_MODE_COMPLETE_XFER);
        contador++;
        vTaskDelay(pdMS_TO_TICKS(1));
	}
	while ((estado != BusPrincipal_MSTR_NO_ERROR)&&(contador<I2C_TIMEOUT));
	if(contador<I2C_TIMEOUT){
        comando_ok = pdTRUE;
    	contador = 0;
        // Espere a que se complete la transferencia de datos
        do{
            estado = BusPrincipal_MasterStatus();
            contador++;
            vTaskDelay(pdMS_TO_TICKS(1));
        }
        while((estado == BusPrincipal_MSTAT_XFER_INP)&&(contador<I2C_TIMEOUT));
        estado = BusPrincipal_MasterClearStatus();
        if(estado & BusPrincipal_MSTAT_ERR_XFER){
            comando_ok = pdFALSE;
        }
        else{
            comando_ok = pdTRUE; 
        }
    }
    BusPrincipal_MasterClearReadBuf();
    vTaskDelay(pdMS_TO_TICKS(10));
    return comando_ok;
}

void bus_limpiar_bufferEscritura(uint8* bufferEscritura, uint8 largo){
    uint8 indice=0;
    BusPrincipal_MasterClearWriteBuf();
    for(indice=0;indice<largo;indice++){
        bufferEscritura[indice]=0;
    }
}

void bus_limpiar_bufferLectura(uint8* bufferLectura, uint8 largo){
    uint8 indice=0;
    BusPrincipal_MasterClearReadBuf();
    for(indice=0;indice<largo;indice++){
        bufferLectura[indice]=0;
    }
}

/* [] END OF FILE */
