 /* ===================================================
 *
 * Copyright EKIA Technology S.A.S, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY S.A.S.
 *
 * AMIGO 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================
*/

/*NOTA: FALTA REALIZAR ACCIONES EN CASO DE ERROR*/

#include <string.h>
#include "stdio.h"
#include "cypins.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "colas.h"
#include "tipos.h"
#include "semaforos.h"
#include "sistema_pago.h"
#include "MDB_monedero.h"
#include "MDB_billetero.h"
#include "LED_ERROR.h"
#include "LED_DEBUG.h"
#include "string.h"
#include "stdio.h"
#include "memoriaExt.h"
#include "MDB.h"
#include "MDB_Tipos.h"

CYBIT banderaRelleno = pdFALSE;
CYBIT banderaMonedero = pdFALSE;
CYBIT banderaBilletero = pdFALSE;
int creditoTemp = 0;

xcoin_id monederoIdent;
xcoin_setup monederoConfig;
xcoin_state monederoStatus;
xcoin_tube monederoTubos;
xbill_state billeteroStatus;
xbill_setup billeteroConfig;
xbill_recycler_status recicladorEstado;

CYBIT led = pdFALSE;
uint8 conteo_error_Billetero = 0;
uint8 conteo_error_Monedero = 0;
uint8 dispositivos = 0; //cada bit, indica un dispositivo, 0 si no está presente, 1 si está conectado
    

/*******************************************************************************
* FUNCIONES DE RESETEO DE DISPOSITIVOS MDB
********************************************************************************
* Funcion de inicialización de los dispositivos MDB
*******************************************************************************/
uint8 reset_monedero(){
    xestadosMonedero monederoEstado;
    
    xQueuePeek(estadoMonedero, (void*)&monederoEstado, 10);
    monederoEstado.monederoOk = NO_LISTO;
    monederoEstado.monedero = MDB_NO_DETECT;
    if(coin_start()){
        conteo_error_Monedero = 0;
        monederoEstado.monederoOk = LISTO;
        monederoEstado.monedero = MDB_DETECT;
        dispositivos = dispositivos | 0x01;
    }    
    xQueueOverwrite(estadoMonedero, (void*)&monederoEstado); 
    //xQueuePeek(setupCoin, &monederoConfig, 10);
    //xQueuePeek(identCoin, &monederoIdent, 10);
    //xQueuePeek(stateCoin, &monederoStatus, 10);
    return monederoEstado.monederoOk;
}

uint8 reset_billetero(){
    xestadosBilletero billeteroEstado;
    
    xQueuePeek(estadoBilletero, (void*)&billeteroEstado, 10);
    billeteroEstado.billeteroOk = NO_LISTO;
    billeteroEstado.billetero = MDB_NO_DETECT;
    if(bill_start()){ //si hay billetero, inicialicelo
        conteo_error_Billetero = 0;
        billeteroEstado.billeteroOk = LISTO;
        billeteroEstado.billetero = MDB_DETECT;
        dispositivos = dispositivos | 0x02;
        xQueuePeek(setupBill,(void*)&billeteroConfig,10);
    }
    xQueueOverwrite(estadoBilletero, (void*)&billeteroEstado);
    return billeteroEstado.billeteroOk;
}

/*******************************************************************************
* FUNCIONES INTERPRETES DE POLL PARA LOS DISPOSITIVOS MDB
********************************************************************************
* Funcion encargada de mantener la comunicación MDB, reportar fallos o eventos 
* de los dispositivos MDB, interpreta los comandos enviados desde los esclavos
* y los reporta de forma organizada para abrir o cerrar sesiones de venta.
*******************************************************************************/
xpollCoin monedero_Poll(){
    xcoin_activity monederoActividad;
    xpollCoin monedero;
    xestadosMonedero monederoEstado;
    uint8_t comando_ok = pdFALSE;
    monedero.ingresadas.cantidad_almacenada = 0;
    monedero.ingresadas.moneda_depositada = 0;
    monedero.ingresadas.ruta = MONEDA_ESPERA;
    monedero.devueltas.monto = 0;
    monedero.cancelar=pdFALSE; 
    sprintf(monedero.ingresadas.ruteo," ");
    
    comando_ok = COIN_poll(&monederoActividad);
    switch(comando_ok){
        case pdFALSE:{
            conteo_error_Monedero++;
            xQueuePeek(estadoMonedero, (void*)&monederoEstado, 10);
            monederoEstado.monedero = MDB_NO_DETECT;
            xQueueOverwrite(estadoMonedero, (void*)&monederoEstado);
        break;}
        case pdTRUE:{
            monedero.comando_ok = pdTRUE;
            MDB_tx_ACK();
            if ((monederoActividad.poll[0]>>7)==1){ //monedas dispensadas manualmente
                monedero.devueltas = moneda_dispensa(monederoActividad.poll[0], monederoActividad.poll[1]);
            }
            else if ((monederoActividad.poll[0]>>6)==1){ //monedas ingresadas
                monedero.ingresadas = moneda_ingreso(monederoActividad.poll[0], monederoActividad.poll[1]);
            }
            if(monederoActividad.poll[0]==1){//se hizo una solicitud del escrow, devuelva el dinero
                monedero.cancelar=pdTRUE;
            }
        break;}
        case EKIA_ACK:{
            monedero.comando_ok = pdTRUE;
            MDB_tx_ACK();
        break;}
        case EKIA_NACK:{
            monedero.comando_ok = pdTRUE;
            MDB_tx_ACK();
        break;}
        default:{
            monedero.comando_ok = pdTRUE;
            MDB_tx_ACK();
            if ((monederoActividad.poll[0]>>7)==1){ //monedas dispensadas manualmente
                monedero.devueltas = moneda_dispensa(monederoActividad.poll[0], monederoActividad.poll[1]);
            }
            else if ((monederoActividad.poll[0]>>6)==1){ //monedas ingresadas
                monedero.ingresadas = moneda_ingreso(monederoActividad.poll[0], monederoActividad.poll[1]);
            }
            if(monederoActividad.poll[0]==1){//se hizo una solicitud del escrow, devuelva el dinero
                monedero.cancelar=pdTRUE;
            }
        break;}
    }
    return monedero;
}

xpollBill billetero_Poll(){
    xbill_activity billeteroActividad;
    xpollBill billetero;
    billetero.comando_ok = pdFALSE;
    billetero.ingresado.billete_depositado=0;
    xestadosBilletero billeteroEstado;
    
    taskENTER_CRITICAL();
    billeteroActividad = BILL_poll();
    
    switch(billeteroActividad.comando_ok){
        case pdFALSE:{
            conteo_error_Billetero++;
            xQueuePeek(estadoBilletero, (void*)&billeteroEstado, 10);
            billeteroEstado.billetero = MDB_NO_DETECT;
            xQueueOverwrite(estadoBilletero, (void*)&billeteroEstado);
        break;}
        case pdTRUE:{
            billetero.comando_ok = pdTRUE;
            MDB_tx_ACK();
            if((billeteroActividad.poll[0]&0x80)==0x80){
                billetero.ingresado = billete_ingreso(billeteroActividad.poll[0]);
            }
        break;}
        case EKIA_ACK:{
            billetero.ingresado.billete_depositado = 0; 
            billetero.comando_ok = pdTRUE;
            MDB_tx_ACK();
            if(BILL_stacker()){
                xQueuePeek(stateBill, (void*)&billeteroStatus,10);
            }
            if(billeteroConfig.nivel==2){
                if(BILL_exp_dispense_status()){
                    xQueuePeek(stateRecycler, (void*)&recicladorEstado,10);
                }
            }
        break;}
        case EKIA_NACK:{
            billetero.ingresado.billete_depositado = 0; 
            billetero.comando_ok = pdTRUE;
            MDB_tx_ACK();
        break;}
        case EKIA_JUST_RESET:{
            MDB_tx_ACK();
            reset_billetero();
        break;}
        default:{
            billetero.comando_ok = pdTRUE;
            MDB_tx_ACK();
            if((billeteroActividad.poll[0]&0x80)==0x80){
                billetero.ingresado = billete_ingreso(billeteroActividad.poll[0]);
            }
        break;}
    }
    taskEXIT_CRITICAL();
    return billetero;
}

/*******************************************************************************
* FUNCIONES DE VENTA Y VALIDACION DE DINERO INGRESADO
********************************************************************************
* Funcion encargadas de validar billetes ingresados, asi como el credito total
* ingresado. 
*******************************************************************************/
xcredito valCreditoMonedas(xcredito credito_anterior, xmoneda_ingresada moneda){
    xcredito credito = credito_anterior;
    xEventos evento;
    if((dispositivos&0x01)==1){
        if((moneda.ruta==MONEDA_CASHBOX)||(moneda.ruta==MONEDA_TUBOS)){
        //if((moneda.moneda_depositada>0)&&(moneda.moneda_depositada<=1000)){
            credito.saldo += moneda.moneda_depositada;
            strcpy(evento.tipo,tipo_operacion);
            sprintf(evento.evento,"%s%d",text_registro_moneda,moneda.moneda_depositada);
            evento.operacion = ACTUALIZAR_REGISTRO;
            xQueueSendToBack(actualizarRegistro,(void*)&evento,5);
        }
    }
    return credito;
}

xcredito valCreditoBillete(xcredito credito_anterior, xbillete_ingresado billete){
    xcredito credito = credito_anterior;
    xEventos evento;
    xConfiguracion parametros;
    
    xQueuePeek(configuracionesMaquina,(void*)&parametros,10);
    if((dispositivos&0x02)==2){
        if(billete.billete_depositado>0){
            if( (billete.ruteo==BILL_STACK) || (billete.ruteo==BILL_RECICLADOR) ){
                credito.saldo += billete.billete_depositado;
                strcpy(evento.tipo,tipo_operacion);
                sprintf(evento.evento,"%s%d",text_registro_billete,billete.billete_depositado);
                evento.operacion = ACTUALIZAR_REGISTRO; 
                xQueueSendToBack(actualizarRegistro,(void*)&evento,200);
            }
        }
    }
    return credito;
}

uint dispensarBilletes(uint cantidadDevolucion){
    xSistemasPago parametros;
    int tipo_billete = 0;
    uint8 indice=0, conteoBilletes=0;
    uint16 devolucionBilletes = 0;
    xbill_paypoll recicladorIntervalo;
    long billetesDevueltos = 0;
    uint16 estadoDevolucion[16];
    xEventos evento;
    xbill_setup confBilletero;
    xQueuePeek(setupBill,(void*)&confBilletero,10);
    
    xQueueOverwrite(creditoValido,&cantidadDevolucion);
    //verifica si el monto a devolver no supera el valor de billetes reciclados y el valor del billete que se recicla
    if(billeteroConfig.nivel==2){//Hay reciclador disponible?
        xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
        if(estado_tambor()>0){ //tiene billetes disponibles para devolver?
            if(parametros.billetesReciclados<=cantidadDevolucion){//si se devuelve al menos un billete no debe superar la cantidad a devolver
                devolucionBilletes = cantidadDevolucion/parametros.billetesReciclados;//paso intermedio para obtener el valor entero de billetes a devolver
                devolucionBilletes = (devolucionBilletes*parametros.billetesReciclados*confBilletero.factorDecimal)/confBilletero.factorEscala;
                if(BILL_exp_dispense_value(devolucionBilletes)){
                    while(parametros.billetesReciclados<=cantidadDevolucion){
                        if(BILL_paypoll()!=EKIA_ACK){
                            xQueueReceive(paypollRecycler,(void*)&recicladorIntervalo,10);
                            if(recicladorIntervalo.actividad!=0){
                                conteoBilletes++;
                                strcpy(evento.tipo,tipo_operacion);
                                sprintf(evento.evento,"%s%d",text_registro_dev_billete,recicladorIntervalo.actividad);
                                evento.operacion = ACTUALIZAR_REGISTRO; 
                                xQueueSendToBack(actualizarRegistro,(void*)&evento,200); 
                            }
                            cantidadDevolucion -= recicladorIntervalo.actividad; 
                            xQueueOverwrite(creditoValido,&cantidadDevolucion);
                        }
                        else if(BILL_payout_status()==pdTRUE){
                            xQueuePeek(payoutRecycler, (void*)&estadoDevolucion,10);
                            for(indice=0;indice<LONGITUD_TIPO_BILLETES;indice++){
                                billetesDevueltos += estadoDevolucion[indice];
                                if(estadoDevolucion[indice]!=0){
                                    tipo_billete = confBilletero.tipo[indice]*confBilletero.factorEscala/confBilletero.factorDecimal;
                                }
                            }
                            if((conteoBilletes+1)==billetesDevueltos){
                                MDB_tx_ACK();
                                strcpy(evento.tipo,tipo_operacion);
                                sprintf(evento.evento,"%s%d",text_registro_dev_billete,tipo_billete);
                                evento.operacion = ACTUALIZAR_REGISTRO; 
                                xQueueSendToBack(actualizarRegistro,(void*)&evento,200); 
                                cantidadDevolucion -= tipo_billete; 
                                xQueueOverwrite(creditoValido,&cantidadDevolucion);
                            }
                        }
                        monedero_Poll();
                    }
                }
            }
        }
    }
    return cantidadDevolucion;
}

xcredito devolver(xcredito credito, int cantidadDevolucion){
    xcredito nuevoCredito;
    xEventos evento;
    uint8 memoriaSD;
    uint16 estadoDevolucion[16];
    uint16 intervaloMonedero = 0;
    uint8 devolucionMonedas = 0;
    uint16 monedasDevueltas = 0;
    uint8 indice = 0;
    xcoin_setup confMonedero;
    CYBIT fin = pdFALSE;
    
    xQueuePeek(setupCoin,(void*)&confMonedero,10);
    xQueuePeek(estadoMemoriaSD, &memoriaSD,100);
    
    strcpy(evento.tipo,tipo_operacion);
    sprintf(evento.evento,"%s%d",text_registro_devolucion,cantidadDevolucion);
    evento.operacion = ACTUALIZAR_REGISTRO; 
    xQueueSendToBack(actualizarRegistro,(void*)&evento,10);
    
    if(credito.depositado.billete_depositado){ //hay un billete?
        if((uint)cantidadDevolucion >= credito.depositado.billete_depositado){ //verifique si el valor a devolver es mayor o igual que el billete retenido, de ser así devuelvalo
            if(BILL_escrow(0x00)){//devuelva el billete y encuentre el monto que haga falta en monedas
                MDB_tx_ACK();
                cantidadDevolucion = cantidadDevolucion-credito.depositado.billete_depositado;
                strcpy(evento.tipo,tipo_operacion);
                sprintf(evento.evento,"%s%d",text_registro_dev_billete,credito.depositado.billete_depositado);
                evento.operacion = ACTUALIZAR_REGISTRO; 
                xQueueSendToBack(actualizarRegistro,(void*)&evento,200);
            }
        }
        else{
            if(BILL_escrow(0xFF)){//el nuevo saldo no alcanza para devolver el billete si es necesario, por lo tanto guardelo
                MDB_tx_ACK();
                strcpy(evento.tipo,tipo_operacion);
                sprintf(evento.evento,"%s%d",text_registro_billete,credito.depositado.billete_depositado);
                evento.operacion = ACTUALIZAR_REGISTRO; 
                xQueueSendToBack(actualizarRegistro,(void*)&evento,200);
            }
        }
    }
    credito.depositado.billete_depositado = pdFALSE; //actualice el estado del billete ingresado
    xQueueOverwrite(creditoValido,&cantidadDevolucion);//actualice el credito en pantalla
    
    //si tiene reciclador devuelva primero billetes
    cantidadDevolucion = dispensarBilletes(cantidadDevolucion);
    
    if((cantidadDevolucion>0)&&(cantidadDevolucion<20000)){
        devolucionMonedas = cantidadDevolucion/monederoConfig.factorEscala*monederoConfig.decimales;
        if(COIN_exp_payout(devolucionMonedas)){
            while(!fin){
                if(COIN_exp_paypoll()!=EKIA_ACK){
                    MDB_tx_ACK();
                    xQueueReceive(paypollCoin,(void*)&intervaloMonedero,10);
                    if((intervaloMonedero!=0)&&(cantidadDevolucion>0)){
                        strcpy(evento.tipo,tipo_operacion);
                        sprintf(evento.evento,"%s%d",text_registro_dev_moneda,intervaloMonedero);
                        evento.operacion = ACTUALIZAR_REGISTRO;
                        xQueueSendToBack(actualizarRegistro,(void*)&evento,200); 
                        cantidadDevolucion -= intervaloMonedero;
                        xQueueOverwrite(creditoValido,&cantidadDevolucion);
                    }
                    else if((cantidadDevolucion<=0)){
                        cantidadDevolucion = 0;
                        fin = pdTRUE;
                    }
                }
                else{
                    if(COIN_exp_paypoll_status()==pdTRUE){
                        MDB_tx_ACK();
                        fin = pdTRUE;
                        xQueuePeek(payoutCoin, (void*)&estadoDevolucion,10);
                        for(indice=0;indice<LONGITUD_TIPO_MONEDAS;indice++){
                            if(estadoDevolucion[indice]!=0){
                                monedasDevueltas += confMonedero.tipo[indice]*estadoDevolucion[indice];
                            }
                        }
                    }
                }
                billetero_Poll();
                vTaskDelay(pdMS_TO_TICKS(3));
            }
        }
    }
    nuevoCredito.saldo = cantidadDevolucion; //calcule el saldo disponible luego de devolver dinero
    //nuevoCredito.saldo=0;
    return nuevoCredito;
}

/*******************************************************************************
* INTERPRETE DE FUNCIONES USB PARA MANEJO DEL MDB
********************************************************************************
* Funcion encargada de interpretar comandos enviados desde el usb
*******************************************************************************/
void funciones_mdb(){
    xbill_state billeteroStatus;
    xcomandoUSB funcionUSB;
    xcomandoHMI funcionHMI;
    int valor_completo;
    uint8 valor;
    xSistemasPago parametros;
    
    xQueuePeek(configuracionSistemasPago,(void*)&parametros,10);
    xQueueReceive(funcion, &funcionUSB, 10);
    xQueueReceive(comandosHMI, &funcionHMI, 10);
    if((funcionUSB.comando == MDB_EKIA_RESETEO)||(funcionHMI.comando == MDB_EKIA_RESETEO)){
        LED_ERROR_Write(0);
        reset_monedero();
        reset_billetero();
        funcionUSB.comando = ESPERA;
        funcionHMI.comando = ESPERA;
    }
    if((funcionUSB.comando == MDB_EKIA_TIPO_BILLETE)||(funcionHMI.comando == MDB_EKIA_TIPO_BILLETE)){
        BILL_type(parametros.billetesAceptados,parametros.billetesAceptados);
        funcionUSB.comando = ESPERA;
        funcionHMI.comando = ESPERA;
    }
    if((funcionHMI.comando == MDB_EKIA_VACIAR_RECICLADOR)&&(funcionHMI.opcional1 == EKIA_ACK)){
        vaciar_reciclador();
        funcionHMI.comando = ESPERA;
    }
    if(funcionHMI.comando == MDB_EKIA_ESTADO_TUBOS){
        COIN_tube_status();
        funcionHMI.comando = ESPERA;
    }
    if(funcionHMI.comando == MDB_EKIA_GUARDAR_BILL){
        BILL_type(parametros.billetesAceptados,parametros.billetesAceptados);
        funcionHMI.comando = ESPERA;
    }
    if(funcionUSB.comando == MDB_EKIA_DISPENSE){
        valor_completo = funcionUSB.opcional1;
        valor = valor_completo/monederoConfig.factorEscala;
        COIN_exp_payout(valor);
        vTaskDelay(pdMS_TO_TICKS(10));
        funcionUSB.comando = ESPERA;
    }
    if(funcionUSB.comando == MDB_EKIA_GUARDAR_BILL){
        BILL_escrow(0xFF);
        funcionUSB.comando = ESPERA;
    }
    if(funcionUSB.comando == MDB_EKIA_DEVOLVER_BILL){
        BILL_escrow(0x00);
        funcionUSB.comando = ESPERA;
    }
    if(funcionUSB.comando == MDB_EKIA_ENABLE_BILL){
        if(BILL_stacker()){
            MDB_tx_ACK();
            xQueuePeek(stateBill, &billeteroStatus, 10);
            BILL_type(parametros.billetesAceptados,parametros.billetesAceptados);
        }
        funcionUSB.comando = ESPERA;
    }
    if(funcionUSB.comando == MDB_EKIA_DISABLE_BILL){
        if(BILL_stacker()){
            MDB_tx_ACK();
            xQueuePeek(stateBill, &billeteroStatus, 10);
            BILL_type(0x0000,0x0000);
        }
        funcionUSB.comando = ESPERA;
    }
}

/*******************************************************************************
* ESTADO DE LOS TUBOS DEL MONEDERO
********************************************************************************
* Esta función verifica el estado de cada tubo del monedero y calcula el monto
* total almacenado y disponible para devolver dinero.
*******************************************************************************/
int estado_tubos(){
    int almacenaje=0;
    uint8 indice=0;
    if(COIN_tube_status()){
        xQueuePeek(tubeCoin, &monederoTubos,10);
        for(indice=0;indice<LONGITUD_TUBOS;indice++){
            almacenaje += monederoTubos.tuboEstado[indice]*monederoConfig.tipo[indice]*monederoConfig.factorEscala/monederoConfig.decimales;
        }
    }
    return almacenaje;
}

/*******************************************************************************
* ESTADO DEL TAMBOR DEL RECICLADOR
********************************************************************************
* Esta función verifica el estado del tambor del reciclador para determinar el 
* valor total de dinero retenido
*******************************************************************************/
long estado_tambor(){
    long almacenaje=0;
    uint8 indice=0;
    int tipo_billete = 0;
    xbill_setup confBilletero;
    
    if(BILL_exp_dispense_status()){
        xQueuePeek(setupBill,(void*)&confBilletero,10);
        xQueuePeek(stateRecycler, (void*)&recicladorEstado,10);
        for(indice=0;indice<LONGITUD_TIPO_BILLETES;indice++){
            tipo_billete = confBilletero.tipo[indice]*confBilletero.factorEscala/confBilletero.factorDecimal;
            almacenaje += recicladorEstado.cantidadBilletes[indice]*tipo_billete;
        }
    }
    return almacenaje;
}

CYBIT vaciar_reciclador(){
    xEventos evento;
    long cantidadReciclada = estado_tambor();
    strcpy(evento.tipo,tipo_operacion);
    sprintf(evento.evento,"%s",text_registro_vacioReciclador);
    evento.operacion = ACTUALIZAR_REGISTRO; 
    xQueueSendToBack(actualizarRegistro,(void*)&evento,200); 
    if(dispensarBilletes(cantidadReciclada)==0){
        return pdTRUE;
    }
    else{
        return pdFALSE;
    }
}

/*******************************************************************************
* FUNCION PRINCIPAL DEL SISTEMA DE PAGO
********************************************************************************
* Funcion encargada de inicializar y mantener la comunicación MDB
*******************************************************************************/
 void Sistemas_pago(void* pvParameters){ 
    (void)pvParameters;
    //uint8 estadoPagoReciclador[32];  
    uint8 MDB;
    CYBIT ledError = pdFALSE;
    xcredito creditoIn;
    xestadosBilletero billetero;
    xestadosMonedero monedero;
    xSistemasPago parametrosSistemasPago;
    xpollCoin pollMonedero;
    xpollBill pollBilletero;
    xresPantalla peticiones = {ESPERA_OPERACION, 0};
    xConfiguracion parametros;
    xEventos evento;
    
    xQueuePeek(estadoBilletero, (void*)&billetero, 100);
    xQueuePeek(estadoMonedero, (void*)&monedero, 100);
    xQueuePeek(estadoMDB, (void*)&MDB, 100);
    xQueuePeek(configuracionSistemasPago,(void*)&parametrosSistemasPago,10);
    
    MDB = LISTO;
    billetero.billetero = MDB_ESPERA;
    monedero.monedero = MDB_ESPERA;
    creditoIn.saldo = 0;
    
    xQueueReceive(respuestaPantalla,(void*)&peticiones,10);
    xQueueOverwrite(estadoBilletero, (void*)&billetero); 
    xQueueOverwrite(estadoMonedero, (void*)&monedero);
    xQueueOverwrite(estadoMDB, (void*)&MDB);
    vTaskDelay(pdMS_TO_TICKS(200));
    reset_monedero();
    reset_billetero();
    for(;;){
        xQueuePeek(configuracionSistemasPago,(void*)&parametrosSistemasPago,10);
        xQueuePeek(configuracionesMaquina, (void*)&parametros, 10);
        xQueuePeek(respuestaPantalla, (void*)&peticiones, 10); //se reciben peticiones realizadas con la pantalla tactil
        if(parametros.estadoMaquina!=ERROR_FUSIBLE){
            if(dispositivos==0){ //si no hay dispositivos MDB detectados se activa el protocolo de error
                LED_ERROR_Write(1);
                vTaskDelay(pdMS_TO_TICKS(2000));
                reset_monedero();
                reset_billetero();
            }        
            else{//si hay dispositivos detectados entonces funciona normalmente
                LED_ERROR_Write(0);
                if((dispositivos&0x01)==1){
                    pollMonedero = monedero_Poll();
                    //COIN_exp_diagnostic_status();
                }
                if(conteo_error_Monedero >= 3){
                    dispositivos = dispositivos & 0xFE;
                    reset_monedero();
                }
                if((dispositivos&0x02)==2){
                    pollBilletero = billetero_Poll();
                }  
                if(conteo_error_Billetero >= 3){
                    dispositivos = dispositivos & 0xFD;
                    reset_billetero();
                }
                switch(peticiones.operacion){
                    case ACTIVAR_SISTEMAS:{
                        if((dispositivos&0x01)==1){
                            COIN_Enable();
                        }
                        if((dispositivos&0x02)==2){
                            BILL_Enable();
                        }
                    break;}
                    case DESACTIVAR_SISTEMAS:{
                        if((dispositivos&0x01)==1){
                            COIN_Disable();
                        }
                        if((dispositivos&0x02)==2){
                            BILL_Disable();
                        }
                    break;}
                    case RELLENO_MONEDAS:{
                        if((dispositivos&0x02)==2){
                            if(billeteroConfig.nivel==1){
                                BILL_Disable();
                            }
                            else{
                                BILL_type(parametrosSistemasPago.billetesAceptados,0);
                            }
                        }
                    break;}
                }
            }
            if((peticiones.operacion == ACTIVAR_SISTEMAS)||(peticiones.operacion == DESACTIVAR_SISTEMAS)){
                peticiones.operacion = ESPERA_OPERACION;
                peticiones.valor = 0;
                xQueueOverwrite(respuestaPantalla,(void*)&peticiones);
            }
            if((peticiones.operacion != RELLENO_MONEDAS)&&(dispositivos>0)){ //si la sesion esta abierta, verifique el monto de monedas almacenadas y sume la cantidad ingresada
                taskENTER_CRITICAL();
                creditoIn = valCreditoMonedas(creditoIn,pollMonedero.ingresadas);
                creditoIn = valCreditoBillete(creditoIn,pollBilletero.ingresado); //actualice el crédito actual
                taskEXIT_CRITICAL();
                switch(peticiones.operacion){
                    case CANCELAR_VENTA:{
                        if(xSemaphoreTake(operacionOcupada,( TickType_t )1)){
                            strcpy(evento.tipo,tipo_operacion);
                            sprintf(evento.evento,"%s%d",text_registro_credito,creditoIn.saldo);
                            evento.operacion = ACTUALIZAR_REGISTRO; 
                            xQueueSendToBack(actualizarRegistro,(void*)&evento,200);
                            peticiones.valor = 0;
                            peticiones.operacion = ESPERA_OPERACION;
                            creditoIn = devolver(creditoIn,creditoIn.saldo);
                            xQueueOverwrite(respuestaPantalla,(void*)&peticiones);
                            xSemaphoreGive(operacionOcupada);   
                        }
                    break;}
                    case MODIFICAR_SALDO:{
                        peticiones.operacion = ESPERA_OPERACION;
                        creditoIn.saldo += peticiones.valor;
                        peticiones.valor = 0;
                        xQueueOverwrite(respuestaPantalla,(void*)&peticiones);
                    break;}
                }
                if(xSemaphoreTake(operacionOcupada,( TickType_t )1)){
                    if(pollMonedero.cancelar==pdTRUE){
                        strcpy(evento.tipo,tipo_operacion);
                        sprintf(evento.evento,"%s%d",text_registro_credito2,creditoIn.saldo);
                        evento.operacion = ACTUALIZAR_REGISTRO; 
                        xQueueSendToBack(actualizarRegistro,(void*)&evento,200);
                        peticiones.valor = 0;
                        creditoIn = devolver(creditoIn,creditoIn.saldo);
                        pollMonedero.cancelar=pdFALSE;
                        xQueueOverwrite(respuestaPantalla,(void*)&peticiones);
                    }
                    xSemaphoreGive(operacionOcupada);   
                }
                if(creditoIn.saldo<=0){
                    peticiones.valor = 0;
                    creditoIn.saldo=0;
                    //xQueueOverwrite(respuestaPantalla,(void*)&peticiones);
                }
            }
            xQueueOverwrite(creditoValido,&creditoIn.saldo);
            funciones_mdb();
            vTaskDelay(pdMS_TO_TICKS(3));
        }
        else{
            BILL_Disable();
            creditoIn = devolver(creditoIn,creditoIn.saldo);
            xQueueOverwrite(creditoValido,&creditoIn.saldo);
            while(1){
                ledError = !ledError;
                LED_ERROR_Write(ledError);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
        }
    }
}

/*******************************************************************************
* FUNCION DE INICIALIZACION DE TAREA MDB
*******************************************************************************/
void pago_Init(void){
    MDB_Start();    //inicializar sistema de comunicación Rs-232 para el MDB
    //Creación de un nuevo task
    if(xTaskCreate(Sistemas_pago, 
                   "Sistemas de pago", 
                   memMDB, 
                   (void*)NULL,
                   priorMDB,
                   (xTaskHandle*)NULL )!= pdPASS)
    {
        for(;;){}
    }
}
/* [] FIN DE ARCHIVO */
