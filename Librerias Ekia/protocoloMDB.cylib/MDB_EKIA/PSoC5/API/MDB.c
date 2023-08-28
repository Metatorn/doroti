/* ===================================================
 *
 * Copyright EKIA Technology SAS, 2021
 * All Rights reserced
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * ALL INFORMATION IS CONFIDENTIAL AND
 * PROPERTARY OF EKIA TECHNOLOGY SAS.
 *
 * Programmed by: Diego Felipe Mejia Ruiz
 * Cali, Colombia
 * ===================================================
;MDB Protocol v4.2
====================================================*/
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"
#include "`$INSTANCE_NAME`_UART.h"
#include "`$INSTANCE_NAME`_monedero.h"
#include "`$INSTANCE_NAME`_billetero.h"
#include "`$INSTANCE_NAME`_isr_rxMDB.h"
#include "`$INSTANCE_NAME`.h"
#include "`$INSTANCE_NAME`_Tipos.h"

uint8 num_dato;
CYBIT finalizado = pdFALSE;

CY_ISR(rxMDB){
    xbufferMDB buffer;
    uint8 status, parity_bit=0;
    #if (MODO_CRITICO)
        BaseType_t uxEstado;
        uxEstado = taskENTER_CRITICAL_FROM_ISR();
    #endif
    xQueuePeekFromISR(bufferRxMDB,(void*)&buffer);
    buffer.ack = pdFALSE;
        
    status = `$INSTANCE_NAME``[UART]`RXSTATUS_REG;
    if(status & `$INSTANCE_NAME``[UART]`RX_STS_MRKSPC){
        parity_bit=1;
    }else{
        parity_bit=0;
    }
    buffer.datosEntrada[num_dato] = `$INSTANCE_NAME``[UART]`GetChar();
    `$INSTANCE_NAME``[isr_rxMDB]`ClearPending();
    if(parity_bit==1){
        finalizado = pdTRUE;
        buffer.finalizado=pdTRUE;
        buffer.indice = num_dato;
        buffer.checksum = buffer.datosEntrada[num_dato];
        if(num_dato==0){//si solo se recibe 1 respuesta
            if(buffer.datosEntrada[num_dato]==`$INSTANCE_NAME`_ACK){ //si es ACK, ok
                buffer.ack = EKIA_ACK;          
            }
            if(buffer.datosEntrada[num_dato]==`$INSTANCE_NAME`_NACK){ //si es NAK, cargar NAK como rta recibida
                buffer.ack = EKIA_NACK;
            }
        }else {  //si se ha recibido mas de 1 respuesta
            buffer.ack = EKIA_MULTI_RESPONSE;
        }
        `$INSTANCE_NAME``[UART]`ClearRxBuffer();
    }
    else{
        finalizado = pdFALSE;
        buffer.finalizado=pdFALSE;
        num_dato++;
    }
        
    #if (MODO_CRITICO)
        taskEXIT_CRITICAL_FROM_ISR(uxEstado);
    #endif
    xQueueOverwriteFromISR(bufferRxMDB, (void*)&buffer,NULL);
}

/*******************************************************************************
* FUNCION DE RECEPCION DE COMANDOS MDB
********************************************************************************
* La función copia todos los bytes recibidos, calcula el checksum y lo compara
* con el recibido, devuelve un 1 si la comunicación es correcta o un 0 si no.
*******************************************************************************/
uint8 `$INSTANCE_NAME`_rx(uint8* MDB_receptBuffer){ //recepción de datos y comandos de MDB
    CYBIT checksum_ok = pdFALSE;
    uint8 check=0;
    int indice=0;
    int timeOut=0, timeOut2 = 0;
    xbufferMDB buffer;
    
    buffer.checksum=0;
    buffer.indice=0;
    buffer.ack=0;
    buffer.finalizado=pdFALSE;
    num_dato=0;
    
    for(indice=0; indice<`$INSTANCE_NAME`_LEN_MAX; indice++){ //limpia el buffer de entrada
        MDB_receptBuffer[indice] = 0;
    }

    while((finalizado==pdFALSE)&&(timeOut<=TMAX)&&(timeOut2<=TIME_OUT)){ //espera a que reciba un dato
        if(num_dato!=0){
            timeOut=0;
            timeOut2++;
        }
        #if(MODO_CRITICO)
            CyDelay(TPERIODO);
        #else    
            vTaskDelay(pdMS_TO_TICKS(TPERIODO)); //si pasa un tiempo sin recibir nada, indica un error
        #endif    
        //xQueuePeek(bufferRxMDB,(void*)&buffer,2);
        timeOut++;
    } 
    num_dato=0;
    xQueueReceive(bufferRxMDB,(void*)&buffer,2);
 
    if((timeOut < TMAX)&&(timeOut2 < TIME_OUT)){
        for(indice=0;indice<buffer.indice;indice++){
            MDB_receptBuffer[indice] = buffer.datosEntrada[indice];
            check += buffer.datosEntrada[indice];
        }
        checksum_ok = buffer.ack;
        if(buffer.ack==EKIA_MULTI_RESPONSE){ //si se recibió varias respuetas
            if(check == buffer.checksum){ //compara los valores, si son iguales, el comando fue correcto
                checksum_ok = pdTRUE;
            }
            else{
                checksum_ok = pdFALSE;
            }
        }
    }
    else{
        //ERROR
        checksum_ok = pdFALSE;
    }
    
    taskENTER_CRITICAL();

    finalizado = pdFALSE;
    xQueueOverwrite(bufferRxMDB,(void*)&buffer);
    taskEXIT_CRITICAL();
    return checksum_ok;
}

/*******************************************************************************
* FUNCION DE TRANSIMISION DE COMANDOS MDB
********************************************************************************
*  
* La función envia los comandos o datos del protocolo MDB, requiere indicar si
* el byte a enviar será de dirección o de dato. Envia automaticamente un checksum
*
*******************************************************************************/
void `$INSTANCE_NAME`_tx(uint8 comando, CYBIT modo){ //rutina para el envio de comandos y datos por MDB
    `$INSTANCE_NAME``[UART]`SetTxAddressMode(modo); //activa o desactiva el bit 9 de acuerdo al modo
    `$INSTANCE_NAME``[UART]`WriteTxData(comando); //envia el comando o el dato MDB
    vTaskDelay(pdMS_TO_TICKS(2));  
}

void `$INSTANCE_NAME`_tx_checksum(uint8 check){
    `$INSTANCE_NAME``[UART]`SetTxAddressMode(`$INSTANCE_NAME``[UART]`SET_SPACE); //envio del checksum
    `$INSTANCE_NAME``[UART]`WriteTxData(check);
}

void `$INSTANCE_NAME`_tx_ACK(){
    `$INSTANCE_NAME`_tx(`$INSTANCE_NAME`_ACK,`$INSTANCE_NAME`_DATO); //envia un ACK
    //tx_checksum(ACK);
    vTaskDelay(pdMS_TO_TICKS(2));
    return;
}

void `$INSTANCE_NAME`_tx_RET(){
    `$INSTANCE_NAME`_tx(`$INSTANCE_NAME`_RET,`$INSTANCE_NAME`_DATO); //envia un RET
    vTaskDelay(pdMS_TO_TICKS(2));
    return;
}

void `$INSTANCE_NAME`_Start(){
    bufferRxMDB = xQueueCreate(1, sizeof(xbufferMDB));
    identCoin = xQueueCreate(1, sizeof(xcoin_id));
    setupCoin = xQueueCreate(1,sizeof(xcoin_setup));
    stateCoin = xQueueCreate(1,sizeof(xcoin_state));
    tubeCoin = xQueueCreate(1,sizeof(xcoin_tube));
    
    setupBill = xQueueCreate(1,sizeof(xbill_setup));
    identBill1 = xQueueCreate(1, sizeof(xbill_id1));
    identBill2 = xQueueCreate(1, sizeof(xbill_id2));
    stateBill = xQueueCreate(1,sizeof(xbill_state));
    typeRecycler = xQueueCreate(1, sizeof(xbill_type_recycler));
    stateRecycler = xQueueCreate(1, sizeof(xbill_recycler_status));
    payoutRecycler = xQueueCreate(1, sizeof(uint16[16]));
    paypollRecycler = xQueueCreate(1, sizeof(xbill_paypoll));
    actividadBilletero = xQueueCreate(1,sizeof(uint8));
    erroresBilletero = xQueueCreate(20,sizeof(uint8));
    avisosBilletero = xQueueCreate(20,sizeof(uint8));
    actividadMonedero = xQueueCreate(1,sizeof(uint16));
    erroresMonedero = xQueueCreate(20,sizeof(uint16));
    avisosMonedero = xQueueCreate(20,sizeof(uint16));
    
    creditoValido = xQueueCreate(1, sizeof(int));
    payoutCoin = xQueueCreate(1,sizeof(uint8[16]));
    paypollCoin = xQueueCreate(1,sizeof(uint16));
    
    estadoMDB = xQueueCreate(1,sizeof(uint8));
    estadoMonedero = xQueueCreate(1,sizeof(xestadosMonedero));
    estadoBilletero = xQueueCreate(1,sizeof(xestadosBilletero));
    configuracionSistemasPago = xQueueCreate(1,sizeof(xSistemasPago));
    
    `$INSTANCE_NAME``[isr_rxMDB]`StartEx(rxMDB);
    `$INSTANCE_NAME``[UART]`Start();    //inicializar sistema de comunicación Rs-232 para el MDB
}


/* [] END OF FILE */
