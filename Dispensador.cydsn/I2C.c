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
#include "tipos.h"
//#include "colas.h"

#include "BusPrincipal.h"
#include "LED_DEBUG.h"

void I2C_habilitar_busPrincipal(uint8* buffer_Lectura, uint8* buffer_Escritura){
    /*inicialice el componente I2C*/
    BusPrincipal_SlaveInitReadBuf(buffer_Lectura,LARGO_BUFFER); //inicie el apuntador a buffer de salida
    BusPrincipal_SlaveInitWriteBuf(buffer_Escritura, LARGO_BUFFER); //inicie el apuntador al buffer de entrada
    BusPrincipal_Start();
    BusPrincipal_SlaveSetAddress(DIRECCION);
}

void I2C_limpiarBuffer(){
    //si termina la respuesta al maestro, limpia el buffer
    if (BusPrincipal_SlaveStatus() & BusPrincipal_SSTAT_RD_CMPLT){
        BusPrincipal_SlaveClearReadBuf();
        (void) BusPrincipal_SlaveClearReadStatus();
    }
}

CYBIT I2C_ComandoRecibido(uint8* buffer_Escritura){
    CYBIT comando_ok = pdFALSE;
    if((BusPrincipal_SlaveStatus() & BusPrincipal_SSTAT_WR_CMPLT)){
        if (LARGO_BUFFER == BusPrincipal_SlaveGetWriteBufSize()){
            // Verifique el inicio y final del paquete para validarlo
            if (buffer_Escritura[PAQUETE_INICIO_POS] == PAQUETE_INICIO){
                comando_ok = pdTRUE;
            }
        }
        // Limpiar buffer de escritura y estado del esclavo 
        BusPrincipal_SlaveClearWriteBuf();
        (void) BusPrincipal_SlaveClearWriteStatus();
    }
    return comando_ok;
}


/* [] END OF FILE */
