 /* ===================================================
 *
 * Copyright EKIA Technologies, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * PROPIEDAD DE EKIA TECH.
 *
 * Billetero 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;Funciones y variables propias del billetero
====================================================*/
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "`$INSTANCE_NAME`_billetero.h"
#include "`$INSTANCE_NAME`.h"
#include "`$INSTANCE_NAME`_Tipos.h"
#include "colas.h"

uint8 datos_entrada[`$INSTANCE_NAME`_LEN_MAX];
xbill_setup billeteroConfig;
xbill_state billeteroStatus;
xbill_id1 billeteroIdent1;
xbill_id2 billeteroIdent2;
xbill_type_recycler recicladorTipo;
xbill_recycler_status recicladorEstado;
xbill_paypoll recicladorIntervalo;

/*******************************************************************************
* FUNCIONES PROPIAS DEL MONEDERO
********************************************************************************
* Funciones varias del monedero, como reset, poll, setup o expansion.
*******************************************************************************/

void enviarAviso(uint8 aviso){
    #if (MDB_BILL_DEBUG)
        xEventos evento;
        sprintf(evento.evento,"Bill_Ad: 0x%X",aviso);
        strcpy(evento.tipo,Bill_Advise);
        evento.operacion = ACTUALIZAR_REGISTRO; 
        xQueueSendToBack(actualizarRegistro,&evento,10);
    #endif
    xQueueSendToBack(avisosBilletero, (void*)&aviso,10);
}

void enviarActividad(uint8 actividad){
    #if (MDB_BILL_DEBUG)
        xEventos evento;
        sprintf(evento.evento,"Bill_Op: 0x%X",actividad);
        strcpy(evento.tipo,Bill_Operation);
        evento.operacion = ACTUALIZAR_REGISTRO; 
        xQueueSendToBack(actualizarRegistro,&evento,10);
    #endif
    xQueueOverwrite(actividadBilletero, (void*)&actividad);
}

void enviarError(uint8 error){
    #if (MDB_BILL_DEBUG)
        xEventos evento;
        sprintf(evento.evento,"Bill_Err: 0x%X",error);
        strcpy(evento.tipo,Bill_Error);
        evento.operacion = ACTUALIZAR_REGISTRO; 
        xQueueSendToBack(actualizarRegistro,&evento,10);
    #endif
    xQueueSendToBack(erroresBilletero, (void*)&error,10);
}

uint8 BILL_reset(){
    vTaskDelay(pdMS_TO_TICKS(5));
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_RESET,`$INSTANCE_NAME`_DIRECCION); // envia un reset
    `$INSTANCE_NAME`_tx_checksum(BILL_RESET);
    taskEXIT_CRITICAL();
    return `$INSTANCE_NAME`_rx(datos_entrada);
}

xbill_activity BILL_poll(){
    uint8 indice = 0;
    xbill_activity actividad;
    actividad.comando_ok=pdFALSE;
    vTaskDelay(pdMS_TO_TICKS(1));
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_POLL,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx_checksum(BILL_POLL);
    taskEXIT_CRITICAL();
    //vTaskDelay(pdMS_TO_TICKS(1));
    actividad.comando_ok = `$INSTANCE_NAME`_rx(datos_entrada);
    if(actividad.comando_ok==pdTRUE){
        for(indice=0;indice<LONGITUD_BILL_POLL;indice++){
            actividad.poll[indice] = datos_entrada[indice];
            switch (datos_entrada[indice]){
                case 0x01: //falla de motor
                    enviarError(datos_entrada[indice]);
                break;
                case 0x02: //falla de algun sensor
                    enviarError(datos_entrada[indice]);
                break;
                case 0x03: //Validador ocupado
                    enviarActividad(datos_entrada[indice]);
                break;
                case 0x04: //falla de checksum
                    enviarError(datos_entrada[indice]);
                break; 
                case 0x05: //billete recibido
                    enviarActividad(datos_entrada[indice]);
                break;
                case 0x06: //validador reseteado
                    actividad.comando_ok = EKIA_JUST_RESET;
                    enviarAviso(datos_entrada[indice]);
                break;    
                case 0x07: //billete retirado o retornado
                    enviarActividad(datos_entrada[indice]);
                break;
                case 0x08: //cajon retirado o abierto
                    enviarAviso(datos_entrada[indice]);
                break; 
                case 0x09: //validador desactivado o inhabilitado
                    //enviarActividad(datos_entrada[indice]);
                break;    
                case 0x0A: //falla de solicitud de scrow
                    enviarError(datos_entrada[indice]);
                break;
                case 0x0B: //billete rechazado por ser desconocido
                    enviarActividad(datos_entrada[indice]);
                break;
                case 0x0C: //se ha intentado retirar un billete acreditado
                    enviarActividad(datos_entrada[indice]);
                break; 
                case 0x21: //solicitud de scrow recibida
                    enviarAviso(datos_entrada[indice]);
                break; 
                case 0x22: //dispensador ocupado activando sistemas de pago
                    enviarActividad(datos_entrada[indice]);
                break; 
                case 0x23: //dispensador ocupado
                    enviarActividad(datos_entrada[indice]);
                break;
                case 0x24: //fallo en sensor de dispensador
                    enviarError(datos_entrada[indice]);
                break;
                case 0x26: //error al arrancar el dispensador/ problema de motor
                    enviarError(datos_entrada[indice]);
                break;  
                case 0x27: //intento de dispensacion resulto en posicion de aceptacion
                    enviarAviso(datos_entrada[indice]);
                break; 
                case 0x28: //error de checksum
                    enviarError(datos_entrada[indice]);
                break; 
                case 0x29: //reciclador desactivado o inhabilitado
                    enviarActividad(datos_entrada[indice]);
                break;
                case 0x2A: //esperando por retirar billete
                    enviarActividad(datos_entrada[indice]);
                break; 
            }
        }
    }
    else{
        for(indice=0;indice<LONGITUD_BILL_POLL;indice++){
            actividad.poll[indice] = 0;
        }
    }
    return actividad;
}

CYBIT BILL_setup(){
    uint8 indice = 0;
    CYBIT func_ok = pdFALSE;
    xSistemasPago parametros;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_SETUP,`$INSTANCE_NAME`_DIRECCION); //envia un SETUP
    `$INSTANCE_NAME`_tx_checksum(BILL_SETUP);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
        xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
        billeteroConfig.nivel = datos_entrada[0];
        billeteroConfig.pais = (datos_entrada[1]<<8) | datos_entrada[2];
        billeteroConfig.factorEscala = ((datos_entrada[3]<<8)|datos_entrada[4])*parametros.factorEscalaBillete;
        billeteroConfig.factorDecimal = pow(10,datos_entrada[5])*parametros.factorDecimalBillete;
        billeteroConfig.capacidad = (datos_entrada[6]<<8)|datos_entrada[7];
        billeteroConfig.nivelSeguridad= (datos_entrada[8]<<8)|datos_entrada[9];
        billeteroConfig.escrow = datos_entrada[10];
        for(indice=0;indice<LONGITUD_TIPO_BILLETES;indice++){
            billeteroConfig.tipo[indice]=datos_entrada[indice+11];
        }
    }
    xQueueOverwrite(setupBill, (void*)&billeteroConfig);
    return func_ok;
}

CYBIT BILL_security(uint8 Y1, uint8 Y2){
    CYBIT func_ok = pdFALSE;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_SECURITY,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(Y1,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y2,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(BILL_SECURITY+Y1+Y2);
    taskEXIT_CRITICAL();
    if (`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
    }
    return func_ok;
}

CYBIT BILL_type(uint16 aceptados, uint16 retenidos){
    CYBIT func_ok = pdFALSE;
    uint8 checksum = 0, Y1, Y2, Y3, Y4;
    Y1 = aceptados>>8;
    Y2 = aceptados & 0xFF;
    Y3 = retenidos>>8;
    Y4 = retenidos & 0xFF;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_TYPE,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(Y1,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y2,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y3,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y4,`$INSTANCE_NAME`_DATO);
    checksum = BILL_TYPE+Y1+Y2+Y3+Y4;
    `$INSTANCE_NAME`_tx_checksum(checksum);
    taskEXIT_CRITICAL();
    if (`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
    }
    return func_ok;
}

CYBIT BILL_escrow(uint8 Y1){
    CYBIT func_ok = pdFALSE;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_ESCROW,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(Y1,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(BILL_ESCROW+Y1);
    taskEXIT_CRITICAL();
    if (`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
    }
    return func_ok;
}

CYBIT BILL_stacker(){
    CYBIT func_ok = pdFALSE;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_STACKER,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx_checksum(BILL_STACKER);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
        billeteroStatus.lleno = datos_entrada[0]>>7;
        billeteroStatus.estado = ((datos_entrada[0]<<8)| datos_entrada[1])&0x7FFF;      
    }
    xQueueOverwrite(stateBill, (void*)&billeteroStatus);
    return func_ok;
}

CYBIT BILL_exp_id_level1(){
    CYBIT func_ok = pdFALSE;
    uint8 indice= 0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION,`$INSTANCE_NAME`_DIRECCION); //envia un EXPANSION/ID
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_LEVEL1,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(BILL_EXPANSION+BILL_EXPANSION_LEVEL1);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
        for(indice=0;indice<3;indice++){
            billeteroIdent1.manufactura[indice]=datos_entrada[indice];
        }
        for(indice=0;indice<12;indice++){
            billeteroIdent1.serie[indice]=datos_entrada[indice+3];
        }
        for(indice=0;indice<12;indice++){
            billeteroIdent2.modelo[indice]=datos_entrada[indice+15];
        }
        billeteroIdent1.version = datos_entrada[27]<<8;
        billeteroIdent1.version |= datos_entrada[28];
        billeteroIdent2.opcional=0;
    }
    xQueueOverwrite(identBill1, (void*)&billeteroIdent1);
    xQueueOverwrite(identBill2, (void*)&billeteroIdent2);
    return func_ok;
}

CYBIT BILL_exp_id_level2(){
    CYBIT func_ok = pdFALSE;
    uint8 indice= 0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION,`$INSTANCE_NAME`_DIRECCION); //envia un EXPANSION/ID
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_LEVEL2_ID,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(BILL_EXPANSION+BILL_EXPANSION_LEVEL2_ID);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
        billeteroIdent2.reciclador = pdFALSE;
        for(indice=0;indice<3;indice++){
            billeteroIdent1.manufactura[indice]=datos_entrada[indice];
        }
        for(indice=0;indice<12;indice++){
            billeteroIdent1.serie[indice]=datos_entrada[indice+3];
        }
        for(indice=0;indice<12;indice++){
            billeteroIdent2.modelo[indice]=datos_entrada[indice+15];
        }
        billeteroIdent1.version = (datos_entrada[27]<<8)|datos_entrada[28];
        for(indice=0;indice<4;indice++){
            billeteroIdent2.opcional|=datos_entrada[indice+29];
            billeteroIdent2.opcional = billeteroIdent2.opcional<<8;
        }
        billeteroIdent2.opcional = billeteroIdent2.opcional>>8;
        if(billeteroIdent2.opcional & 0x02){
            billeteroIdent2.reciclador = pdTRUE;
        }
    }
    xQueueOverwrite(identBill1, (void*)&billeteroIdent1);
    xQueueOverwrite(identBill2, (void*)&billeteroIdent2);
    return func_ok;
}

CYBIT BILL_exp_level2_enable(uint32 funcionesOpcionales){
    uint8 checksum;
    uint8 Y4 = funcionesOpcionales & 0x00FF;
    uint8 Y3 = (funcionesOpcionales>>8) & 0x00FF;
    uint8 Y2 = (funcionesOpcionales>>16) & 0x00FF;
    uint8 Y1 = (funcionesOpcionales>>24) & 0x00FF;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_LEVEL2_ENABLE,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y1,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y2,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y3,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y4,`$INSTANCE_NAME`_DATO);
    checksum = BILL_EXPANSION+BILL_EXPANSION_LEVEL2_ENABLE+Y1+Y2+Y3+Y4;
    `$INSTANCE_NAME`_tx_checksum(checksum);
    taskEXIT_CRITICAL();
    return `$INSTANCE_NAME`_rx(datos_entrada);
}

CYBIT BILL_exp_recycler_setup(){
    CYBIT func_ok = pdFALSE; 
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION, `$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_RECYCLER_SETUP, `$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(BILL_EXPANSION+BILL_EXPANSION_RECYCLER_SETUP);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
        recicladorTipo.tipo = (datos_entrada[0]<<8)|datos_entrada[1];
    }
    xQueueOverwrite(typeRecycler, (void*)&recicladorTipo);
    return func_ok;
}

CYBIT BILL_exp_recycler_enable(char* habilitador){
    uint8 indice = 0;
    uint8 checksum = 0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION, `$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_RECYCLER_ENABLE, `$INSTANCE_NAME`_DATO);
    checksum = BILL_EXPANSION+BILL_EXPANSION_RECYCLER_ENABLE;
    for(indice=0; indice<18; indice++){
        `$INSTANCE_NAME`_tx(habilitador[indice], `$INSTANCE_NAME`_DATO);
        checksum += habilitador[indice];
    }
    `$INSTANCE_NAME`_tx_checksum(checksum);
    taskEXIT_CRITICAL();
    return `$INSTANCE_NAME`_rx(datos_entrada);
}

CYBIT BILL_exp_dispense_status(){
    CYBIT func_ok = pdFALSE;
    uint8 indice = 0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION, `$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_DISPENSE_STATUS, `$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(BILL_EXPANSION+BILL_EXPANSION_DISPENSE_STATUS);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
        recicladorEstado.recicladorLleno = (datos_entrada[0]<<8)|datos_entrada[1];
        for(indice=0;indice<16;indice++){
            recicladorEstado.cantidadBilletes[indice]=(datos_entrada[(indice*2)+2]<<8)|datos_entrada[(indice*2)+3];
        }
    }
    xQueueOverwrite(stateRecycler, (void*)&recicladorEstado);
    return func_ok;
}

CYBIT BILL_exp_dispense_bill(uint8 tipo, uint16 cantidad){
    uint8 checksum;
    uint8 Y2 = cantidad>>8; 
    uint8 Y3 = cantidad & 0xFF; 
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_DISPENSE_BILL,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(tipo,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y2,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y3,`$INSTANCE_NAME`_DATO);
    checksum=BILL_EXPANSION+BILL_EXPANSION_DISPENSE_BILL+tipo+Y2+Y3;
    `$INSTANCE_NAME`_tx_checksum(checksum);
    taskEXIT_CRITICAL();
    return `$INSTANCE_NAME`_rx(datos_entrada);
}

CYBIT BILL_exp_dispense_value(uint16 valor){
    uint8 checksum;
    uint8 Y1 = valor >> 8;
    uint8 Y2 = valor & 0xFF;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_DISPENSE_VALUE,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y1,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y2,`$INSTANCE_NAME`_DATO);
    checksum=BILL_EXPANSION+BILL_EXPANSION_DISPENSE_VALUE+Y1+Y2;
    `$INSTANCE_NAME`_tx_checksum(checksum);
    taskEXIT_CRITICAL();
    return `$INSTANCE_NAME`_rx(datos_entrada);
}

CYBIT BILL_payout_status(){
    CYBIT func_ok = pdFALSE;
    uint8 indice = 0;
    uint16 estado[16];
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION, `$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_PAYOUT_STATUS, `$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(BILL_EXPANSION+BILL_EXPANSION_PAYOUT_STATUS);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){   
        func_ok=pdTRUE;
        for(indice=0; indice<16; indice++){
            estado[indice]=(datos_entrada[indice*2]<<8)|datos_entrada[(indice*2)+1];  
        }
    }
    xQueueOverwrite(payoutRecycler, (void*)&estado);
    return func_ok;
}

CYBIT BILL_paypoll(){
    CYBIT func_ok = pdFALSE;
    xbill_setup confBilletero;
    xQueuePeek(setupBill,(void*)&confBilletero,10);
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION, `$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_PAYOUT_POLL, `$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(BILL_EXPANSION+BILL_EXPANSION_PAYOUT_POLL);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){  
        func_ok=pdTRUE;
        recicladorIntervalo.actividad = (((datos_entrada[0]<<8)|datos_entrada[1])*confBilletero.factorEscala)/confBilletero.factorDecimal;
    }
    xQueueOverwrite(paypollRecycler, (void*)&recicladorIntervalo);
    return func_ok;
}

CYBIT BILL_payout_cancel(){
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(BILL_EXPANSION, `$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(BILL_EXPANSION_PAYOUT_CANCEL, `$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(BILL_EXPANSION+BILL_EXPANSION_PAYOUT_CANCEL);
    taskEXIT_CRITICAL();
    return `$INSTANCE_NAME`_rx(datos_entrada);  
}

xbillete_ingresado billete_ingreso(uint8 ruteoBillete){
    xbillete_ingresado transaccion;
    xbill_setup confBilletero;
    uint8 tipo;
    
    transaccion.ruteo = (ruteoBillete & 0x70)>>4;
    tipo = ruteoBillete & 0x0F;
    xQueuePeek(setupBill,(void*)&confBilletero,10);
    transaccion.billete_depositado = confBilletero.tipo[tipo]*confBilletero.factorEscala/confBilletero.factorDecimal;
    /*switch (ruteo){
        case BILL_STACK:
            NULL;
            char str[]="GUARDADO";
            strcpy(transaccion.ruteo,str);
        break;
        case BILL_RETENIDO:
            NULL;
            char str1[]="RETENIDO";
            strcpy(transaccion.ruteo,str1);
        break;
        case BILL_RETORNADO:
            NULL;
            char str2[]="RETORNADO";
            strcpy(transaccion.ruteo,str2);
        break;
        case BILL_RECICLADOR:
            NULL;
            char str3[]="RECICLADO";
            strcpy(transaccion.ruteo,str3);
        break;
        case BILL_DESHABILITADO_RECHAZADO:
            NULL;
            char str4[]="RECHAZADO";
            strcpy(transaccion.ruteo,str4);
        break;
        case BILL_RECICLADO_MANUAL:
            NULL;
            char str5[]="LLENADO_MANUAL";
            strcpy(transaccion.ruteo,str5);
        break;
        case BILL_DISPENSADO_MANUAL:
            NULL;
            char str6[]="DISPENSADO";
            strcpy(transaccion.ruteo,str6);
        break;
        case BILL_RECICLADOR_A_STACK:
            NULL;
            char str7[]="RECICLADO_GUARDADO";
            strcpy(transaccion.ruteo,str7);
        break;
        default:
            NULL;
            char str8[]="NO_RECONOCIDO";
            strcpy(transaccion.ruteo,str8);
        break;
    }*/
    return transaccion;
}

CYBIT BILL_Disable(){
    CYBIT comando_ok = pdTRUE;
    if(BILL_stacker()){
        `$INSTANCE_NAME`_tx_ACK();
        if(!BILL_type(0x0000,0x0000)){
            comando_ok = pdFALSE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

CYBIT BILL_Enable(){
    CYBIT comando_ok = pdTRUE;
    xSistemasPago parametros;
    xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
    if(BILL_stacker()){
        `$INSTANCE_NAME`_tx_ACK();
        if(!BILL_type(parametros.billetesTrabajo,0x0000)){
            comando_ok = pdFALSE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* FUNCION DE INICIALIZACION DEL BILLETERO
********************************************************************************
* Funcion encargada de inicializar y habilitar el billetero
*******************************************************************************/
CYBIT bill_start(){
    CYBIT comando_ok=0;
    uint8 serie = 1;
    uint8 estado = `$INSTANCE_NAME`_EKIA_RESETEO;
    int timeout = 0;
    uint8 index = 0;
    char billetesReciclador[18] = {0xFF,0xFF,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0};
    xSistemasPago parametros;
    xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
    for(index = 0; index < 16; index++){
        billetesReciclador[index+2] = 3 * ((parametros.billetesReciclados>>index)&0x01);
    }
    while (estado == `$INSTANCE_NAME`_EKIA_RESETEO){
        switch(serie){
            case 1:{ // envia un reset
                if(BILL_reset()==EKIA_ACK){
                    serie++;//se envio correctamente
                }
                else{
                    serie = 0;
                }
                //`$INSTANCE_NAME`_tx_ACK();
            break;}
        
            case 2:{
                do{
                    vTaskDelay(pdMS_TO_TICKS(1));    
                    comando_ok = BILL_poll().comando_ok; //envia un POLL hasta que reciba un JUST_RESET
                    timeout++;
                }while((comando_ok!=EKIA_JUST_RESET)&&(timeout<TMAX_BILL));
                if(timeout>=TMAX_BILL){
                    serie = 0;
                }
                else{
                    serie = 3;
                }
            break;}
                
            case 3:{   
                `$INSTANCE_NAME`_tx_ACK(); //envia un ACK
                serie = BILL_setup()*4; //envia un SETUP
            break;}  
                
            case 4:{   
                `$INSTANCE_NAME`_tx_ACK(); //envia un ACK
                //serie = BILL_security(billeteroConfig.nivelSeguridadAlto,billeteroConfig.nivelSeguridadBajo)*5; //envia un SECURITY
                serie = BILL_security(0,0)*5; //envia un SECURITY
            break;}     
            
            case 5:{    
                `$INSTANCE_NAME`_tx_ACK(); //envia un ACK 
                if(billeteroConfig.nivel==1){
                    serie = BILL_exp_id_level1()*9; //envia un EXPANSION/ID nivel 1
                }
                else{
                    if(parametros.recicladorHabilitado==pdTRUE){
                        serie = 6; 
                    }
                    else{
                        serie = BILL_exp_id_level1()*9; //NOTA: Pueden haber billeteros nivel 2 que no aceptan comando nivel 2
                    }
                }
            break;}
                
            case 6:{   
                `$INSTANCE_NAME`_tx_ACK(); //envia un ACK
                serie = BILL_exp_level2_enable(0x02)*7; //habilita el reciclador
            break;}
            
            case 7:{
                `$INSTANCE_NAME`_tx_ACK();
                serie = BILL_exp_recycler_setup()*8;
            break;}    
                
            case 8:{
                `$INSTANCE_NAME`_tx_ACK();
                serie = BILL_exp_recycler_enable(billetesReciclador)*9;
            break;}    
                
            case 9:{ 
                `$INSTANCE_NAME`_tx_ACK();
                serie = BILL_stacker()*10; //envía un STACKER
            break;}
                
            case 10:{  
                `$INSTANCE_NAME`_tx_ACK();
                serie = BILL_type(parametros.billetesTrabajo,0x0000)*11; //envia un BILL_type
                vTaskDelay(pdMS_TO_TICKS(1));
            break;}
            
            case 11:{
                `$INSTANCE_NAME`_tx_ACK();
                serie = BILL_escrow(0x00)*12;
            break;}    
                
            case 12:{    
                `$INSTANCE_NAME`_tx_ACK();
                comando_ok = pdTRUE; //si ejecutó todos los pasos correctamente, finaliza la inicialización
                estado = `$INSTANCE_NAME`_EKIA_OK;
            break;}               
                
            default:{ //error
                //si ocurrió un error, lo reporta y reinicia la inicialización desde el principio
                //vTaskDelay(100/portTICK_PERIOD_US);
                vTaskDelay(pdMS_TO_TICKS(1));
                comando_ok = pdFALSE;
                estado = `$INSTANCE_NAME`_EKIA_OK;
                serie = 1;
            break;}    
        }
    }
    return comando_ok;
}
/* [] END OF FILE */
