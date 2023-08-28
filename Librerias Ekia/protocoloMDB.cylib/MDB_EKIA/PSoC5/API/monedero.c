/* ===================================================
 *
 * Copyright EKIA Technology SAS, 2021
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * PROPIEDAD DE EKIA TECHNOLOGY SAS.
 *
 * Monedero EKIA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ===================================================
;Funciones y variables propias del monedero
====================================================*/
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cypins.h"
#include "`$INSTANCE_NAME`.h"
#include "LED_ERROR.h"
#include "`$INSTANCE_NAME`_monedero.h"
#include "`$INSTANCE_NAME`_Tipos.h"
#include "math.h"

uint8 datos_entrada[`$INSTANCE_NAME`_LEN_MAX];
xcoin_setup monederoConfig;
xcoin_state monederoStatus;
xcoin_tube monederoTubos;
xcoin_id monederoIdent;

/*******************************************************************************
* FUNCIONES PROPIAS DEL MONEDERO
********************************************************************************
* Funciones varias del monedero, como reset, poll, setup o expansion.
*******************************************************************************/
void enviarAvisoMonedero(uint16 aviso){
    xQueueSendToBack(avisosMonedero, (void*)&aviso,10);
}

void enviarActividadMonedero(uint16 actividad){
    xQueueOverwrite(actividadMonedero, (void*)&actividad);
}

void enviarErrorMonedero(uint16 error){
    xQueueSendToBack(erroresMonedero, (void*)&error,10);
}

uint8 COIN_reset(){
    CYBIT func_ok = 0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_RESET,`$INSTANCE_NAME`_DIRECCION); // envia un reset
    `$INSTANCE_NAME`_tx_checksum(COIN_RESET);
    taskEXIT_CRITICAL();
    func_ok = `$INSTANCE_NAME`_rx(datos_entrada);
    return func_ok;
}

/**************************************************
 * @brief Poll function for coin acceptor response
 * 
 * @param xcoin_activity pointer of the buffer
 **************************************************/
uint8 COIN_poll(xcoin_activity* activity){
    uint8 indice = 0;
    CYBIT command_ok = pdFALSE;
    vTaskDelay(pdMS_TO_TICKS(1));
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_POLL,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx_checksum(COIN_POLL);
    vTaskDelay(pdMS_TO_TICKS(10));
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){  
        command_ok=pdTRUE;
        for(indice = 0; indice < LONGITUD_COIN_POLL; indice++){
            activity->poll[indice]=datos_entrada[indice];
            switch (datos_entrada[indice]){
                case 0x01: //solicitud de escrow para devolver
                    enviarActividadMonedero(datos_entrada[indice]);
                break;
                case 0x02: //monedero ocupado activando dispositivos de pago
                    enviarAvisoMonedero(datos_entrada[indice]);
                break;
                case 0x03: //sin credito, moneda validada
                    enviarActividadMonedero(datos_entrada[indice]);
                break;
                case 0x04: //sensor de tubo defectuoso
                    enviarErrorMonedero(datos_entrada[indice]);
                break;
                case 0x05: //Doble llegada de moneda al mismo tiempo
                    enviarActividadMonedero(datos_entrada[indice]);
                break;
                case 0x06: //Aceptador Desconectado
                    enviarAvisoMonedero(datos_entrada[indice]);
                break;
                case 0x07: //Tubo atascado al intentar dispensar
                    enviarErrorMonedero(datos_entrada[indice]);
                break;    
                case 0x08: //error de checksum interno
                    enviarErrorMonedero(datos_entrada[indice]);
                break;    
                case 0x09: //Error enrutando moneda validada
                    enviarErrorMonedero(datos_entrada[indice]);
                break;    
                case 0x0A: //Monedero ocupado para atender solicitud
                    enviarAvisoMonedero(datos_entrada[indice]);
                break;    
                case 0x0B: //validador reseteado
                    command_ok = EKIA_JUST_RESET;
                    enviarAvisoMonedero(datos_entrada[indice]);
                break;
                case 0x0C: //monedas atascadas en la entrada del monedero
                    enviarErrorMonedero(datos_entrada[indice]);
                break; 
                case 0x0D: //Posible intento de remoción de monedas
                    enviarAvisoMonedero(datos_entrada[indice]);
                break;     
            }
        }   
    }
    else{
        for(indice = 0; indice < LONGITUD_COIN_POLL; indice++){
            activity->poll[indice]=0;
        }
    }
    return command_ok;
}

CYBIT COIN_setup(){
    CYBIT func_ok = pdFALSE;
    uint8 indice = 0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_SETUP,`$INSTANCE_NAME`_DIRECCION); //envia un SETUP
    `$INSTANCE_NAME`_tx_checksum(COIN_SETUP);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada))
    {
        func_ok = pdTRUE;
        monederoConfig.nivel = datos_entrada[0];
        monederoConfig.pais = (datos_entrada[1]<<8) | datos_entrada[2];
        monederoConfig.factorEscala = datos_entrada[3];
        monederoConfig.decimales = pow(10,datos_entrada[4]);
        monederoConfig.ruteo = (datos_entrada[5]<<8)|datos_entrada[6];
        for(indice=0;indice<LONGITUD_TIPO_MONEDAS;indice++){
            monederoConfig.tipo[indice]=datos_entrada[indice+7];
        }
    }
    xQueueOverwrite(setupCoin, (void*)&monederoConfig);
    return func_ok;
}

CYBIT COIN_tube_status(){
    CYBIT func_ok = pdFALSE;
    uint8 indice = 0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_TUBE_STATUS,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx_checksum(COIN_TUBE_STATUS);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
        monederoTubos.tuboLleno = datos_entrada[0]<<8|datos_entrada[1];
        for(indice=0;indice<LONGITUD_TUBOS;indice++){
            monederoTubos.tuboEstado[indice] = datos_entrada[indice+2];
        }      
    }
    xQueueOverwrite(tubeCoin, (void*)&monederoTubos);
    return func_ok;
}

CYBIT COIN_dispense(uint8 tipo, uint8 cantidad){
    uint8 Y1 = (cantidad & 0x0F)<<4;
    Y1 |= tipo & 0xF0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_DISPENSE, `$INSTANCE_NAME`_DIRECCION); //envia un comando DISPENSE
    `$INSTANCE_NAME`_tx(Y1, `$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(COIN_DISPENSE+Y1);
    taskEXIT_CRITICAL();
    return `$INSTANCE_NAME`_rx(datos_entrada);
}

CYBIT COIN_type(uint16 aceptados, uint16 dispensadoManual){
    uint8 checksum = 0, Y1, Y2, Y3, Y4;
    Y1 = aceptados>>8;
    Y2 = aceptados & 0xFF;
    Y3 = dispensadoManual>>8;
    Y4 = dispensadoManual & 0xFF;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_TYPE,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(Y1,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y2,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y3,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y4,`$INSTANCE_NAME`_DATO);
    checksum = COIN_TYPE+Y1+Y2+Y3+Y4;
    `$INSTANCE_NAME`_tx_checksum(checksum);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        return pdTRUE;
    }
    return pdFALSE;
}

CYBIT COIN_exp_id(){
    CYBIT func_ok = 0;
    uint8 indice=0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_EXPANSION,`$INSTANCE_NAME`_DIRECCION); //envia un EXPANSION/ID
    `$INSTANCE_NAME`_tx(COIN_EXPANSION_ID,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(COIN_EXPANSION+COIN_EXPANSION_ID);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
        for(indice=0;indice<3;indice++){
            monederoIdent.manufactura[indice]=datos_entrada[indice];
        }
        for(indice=0;indice<12;indice++){
            monederoIdent.serie[indice]=datos_entrada[indice+3];
        }
        for(indice=0;indice<12;indice++){
            monederoIdent.modelo[indice]=datos_entrada[indice+15];
        }
        monederoIdent.version = datos_entrada[27]<<8;
        monederoIdent.version |= datos_entrada[28];
        monederoIdent.opcional=datos_entrada[29]<<24;
        monederoIdent.opcional|=datos_entrada[30]<<16;
        monederoIdent.opcional|=datos_entrada[31]<<8;
        monederoIdent.opcional|=datos_entrada[32];
    }
    xQueueOverwrite(identCoin, (void*)&monederoIdent);
    return func_ok;
}

CYBIT COIN_exp_feature_en(uint32 funcionesOpcionales){
    uint8 checksum = 0;
    uint8 b0 = funcionesOpcionales & 0x00FF;
    uint8 b1 = (funcionesOpcionales>>8) & 0x00FF;
    uint8 b2 = (funcionesOpcionales>>16) & 0x00FF;
    uint8 b3 = (funcionesOpcionales>>24) & 0x00FF;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_EXPANSION,`$INSTANCE_NAME`_DIRECCION); //envia un EXPANSION ENABLE
    `$INSTANCE_NAME`_tx(COIN_EXPANSION_FEATURE,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(b3,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(b2,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(b1,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(b0,`$INSTANCE_NAME`_DATO);
    checksum = COIN_EXPANSION+COIN_EXPANSION_FEATURE+b3+b2+b1+b0;
    `$INSTANCE_NAME`_tx_checksum(checksum);
    taskEXIT_CRITICAL();
    return `$INSTANCE_NAME`_rx(datos_entrada);
}

uint8 COIN_exp_payout(uint8 Y1){
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_EXPANSION, `$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(COIN_EXPANSION_PAYOUT, `$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx(Y1, `$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(COIN_EXPANSION+COIN_EXPANSION_PAYOUT+Y1);
    taskEXIT_CRITICAL();
    return `$INSTANCE_NAME`_rx(datos_entrada);   
}

CYBIT COIN_exp_diagnostic_status(){
    CYBIT func_ok = 0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_EXPANSION,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(COIN_EXPANSION_SEND_DIAGNOS,`$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(COIN_EXPANSION+COIN_EXPANSION_SEND_DIAGNOS);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok = pdTRUE;
        monederoStatus.principal = datos_entrada[0]<<8;
        monederoStatus.secundario = datos_entrada[1];
        switch (monederoStatus.principal){
            case 0x01:/*iniciando sistema*/{
                enviarAvisoMonedero(monederoStatus.principal);
            break;}
            case 0x02:/*apagando sistema*/{
                enviarAvisoMonedero(monederoStatus.principal);
            break;}
            case 0x03:/*monedero OK*/{
                enviarAvisoMonedero(monederoStatus.principal);
            break;} 
            case 0x04:/*tecla MODE del teclado presionada*/{
                enviarActividadMonedero(monederoStatus.principal);
            break;}
            case 0x05:{
                enviarAvisoMonedero((monederoStatus.principal)|(monederoStatus.secundario));
            break;} 
            case 0x06:/*Inhibido por el VMC*/{
                enviarAvisoMonedero(monederoStatus.principal);
            break;}
            case 0x0A:/*Errores generales*/{
                enviarErrorMonedero((monederoStatus.principal)|(monederoStatus.secundario));
            break;}
            case 0x0B:/*Errores modulo discriminador*/{
                enviarErrorMonedero((monederoStatus.principal)|(monederoStatus.secundario));
            break;}
            case 0x0C:/*Errores puerta de aceptacion*/{
                enviarErrorMonedero((monederoStatus.principal)|(monederoStatus.secundario));
            break;}
            case 0x0D:/*Errores modulo separador*/{
                enviarErrorMonedero((monederoStatus.principal)|(monederoStatus.secundario));
            break;}
            case 0x0E:/*Errores modulo dispensador*/{
                enviarErrorMonedero((monederoStatus.principal)|(monederoStatus.secundario));
            break;}
            case 0x0F:/*Errores cassette de monedas*/{
                enviarErrorMonedero((monederoStatus.principal)|(monederoStatus.secundario));
            break;}
        } 
    }
    xQueueOverwrite(stateCoin, (void*)&monederoStatus);
    return func_ok;
}

CYBIT COIN_exp_paypoll(){
    CYBIT func_ok = pdFALSE;
    uint16 intervalo = 0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_EXPANSION,`$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(COIN_EXPANSION_PAYOUT_POLL, `$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(COIN_EXPANSION+COIN_EXPANSION_PAYOUT_POLL);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){  
        func_ok=pdTRUE;
        intervalo = (datos_entrada[0]*monederoConfig.factorEscala)/monederoConfig.decimales;
        xQueueOverwrite(paypollCoin, (void*)&intervalo);
    }
    return func_ok;
}

CYBIT COIN_exp_paypoll_status(){
    CYBIT func_ok = pdFALSE;
    uint8 estado[16];
    uint8 indice = 0;
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_tx(COIN_EXPANSION, `$INSTANCE_NAME`_DIRECCION);
    `$INSTANCE_NAME`_tx(COIN_EXPANSION_PAYOUT_STATUS, `$INSTANCE_NAME`_DATO);
    `$INSTANCE_NAME`_tx_checksum(COIN_EXPANSION+COIN_EXPANSION_PAYOUT_STATUS);
    taskEXIT_CRITICAL();
    if(`$INSTANCE_NAME`_rx(datos_entrada)){
        func_ok=pdTRUE;
        for(indice=0; indice<16; indice++){
            estado[indice]=datos_entrada[indice];  
        }
    }
    xQueueOverwrite(payoutCoin, (void*)&estado);
    return func_ok;
}

CYBIT COIN_Disable(){
    return COIN_type(0x00,0x00);
}

CYBIT COIN_Enable(){
    return COIN_type(0xFFFF,monederoConfig.ruteo);
}

xmonedas_devueltas moneda_dispensa(uint8 manejoMonedas, uint8 cantidad){
    xmonedas_devueltas transaccion;
    uint8 dispensadas, almacenadas, tipo;
    int moneda;
    dispensadas = manejoMonedas & 0x70; //almacena la cantidad de monedas dispensadas manualmente
    tipo = manejoMonedas & 0x0F; //obtiene el tipo de moneda dispensada
    almacenadas = cantidad; //numero de monedas que quedan en el tubo
    
    moneda = monederoConfig.tipo[tipo]*monederoConfig.factorEscala/monederoConfig.decimales;
    transaccion.monto = dispensadas*moneda;
    transaccion.monto_almacenado = almacenadas*moneda;
    return transaccion;
}

xmoneda_ingresada moneda_ingreso(uint8 manejoMonedas, uint8 cantidad){
    xmoneda_ingresada transaccion;
    uint8 tipo;
    
    transaccion.ruta = (manejoMonedas & 0x30)>>4;
    tipo = manejoMonedas & 0x0F;
    
    transaccion.moneda_depositada = monederoConfig.tipo[tipo]*monederoConfig.factorEscala/monederoConfig.decimales;
    transaccion.cantidad_almacenada = transaccion.moneda_depositada*cantidad;

    switch(transaccion.ruta){
        case MONEDA_CASHBOX:
            NULL;
            char str[]="GUARDADO_EN_CAJA";
            strcpy(transaccion.ruteo,str);
        break;
        case MONEDA_TUBOS:
            NULL;
            char str1[]="GUARDADO_EN_TUBOS";
            strcpy(transaccion.ruteo,str1);
        break;
        case MONEDA_RECHAZADA:
            NULL;
            char str2[]="RECHAZADA";
            strcpy(transaccion.ruteo,str2);
        break;    
        default:
            NULL;
            char str3[]="NO_USADO";
            strcpy(transaccion.ruteo,str3);
        break;    
    }
    return transaccion;
}

/*******************************************************************************
* FUNCION DE INICIALIZACION DEL MONEDERO
********************************************************************************
* Funcion encargada de inicializar y habilitar el monedero
*******************************************************************************/
CYBIT coin_start(){
    CYBIT comando_ok;
    uint8 serie = 1;
    uint8 estado = `$INSTANCE_NAME`_EKIA_RESETEO;
    int timeout=0;
    while (estado == `$INSTANCE_NAME`_EKIA_RESETEO){
        switch(serie){
            case 1: // envia un reset
                if(COIN_reset()==EKIA_ACK){
                    serie++; //se envio correctamente
                }
                else{
                    serie = 0;
                }
            break;
        
            case 2:
                do{   
                    comando_ok = COIN_poll(NULL); //envia un POLL hasta que reciba un JUST_RESET
                    timeout++;
                    vTaskDelay(pdMS_TO_TICKS(1));
                }while((comando_ok!= EKIA_JUST_RESET)&&(timeout<TMAX_COIN));
                if(timeout>=TMAX_COIN){
                    serie = 0;
                }
                else{
                    serie++;
                }
            break;
                
            case 3:
                `$INSTANCE_NAME`_tx_ACK();
                serie = COIN_setup()*4; //envia un SETUP
            break;    
            
            case 4:   
                `$INSTANCE_NAME`_tx_ACK(); //envia un ACK 
                serie = COIN_exp_id()*5; //envia un EXPANSION/ID
            break;
                
            case 5:    
                `$INSTANCE_NAME`_tx_ACK(); //envia un ACK
                comando_ok = COIN_exp_feature_en(0x01); //envia un EXPANSION FEATURE ENABLE
                if(datos_entrada[0]==0){
                    serie = 6;
                }
                else{
                    serie = 0;
                }
            break;
                
            case 6: 
                //`$INSTANCE_NAME`_tx_ACK();
                serie = COIN_exp_diagnostic_status()*7; //envía un EXPANSION SEND DIAGNOSTIC STATUS
            break;
                
            case 7:
                //`$INSTANCE_NAME`_tx_ACK();
                vTaskDelay(pdMS_TO_TICKS(300));
                serie = COIN_tube_status()*8; //envia un EXPANSION TUBE STATUS
            break;
                
            case 8: 
                //LED_DEBUG_Write(1);
                serie = COIN_type(0xFFFF,monederoConfig.ruteo)*9; //envia un COIN TYPE
            break;
                
            case 9:
                comando_ok = pdTRUE; //si ejecutó todos los pasos correctamente, finaliza la inicialización
                estado = `$INSTANCE_NAME`_EKIA_OK;
            break;
                
            default: //error
                //si ocurrió un error, lo reporta y reinicia la inicialización desde el principio
                vTaskDelay(pdMS_TO_TICKS(1));
                comando_ok = pdFALSE;
                estado = `$INSTANCE_NAME`_EKIA_OK;
                serie = 1;
            break;    
        }
    }
    return comando_ok;
}
/* [] END OF FILE */
