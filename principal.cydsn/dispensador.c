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
;Funcoones comunes para el manejo del dispensador
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
#include "dispensador.h"
#include "colas.h"
#include "tipos.h"
#include "semaforos.h"

#include "LED_DEBUG.h"

static uint8 dispensador_buffer_Escritura [LARGO_BUFFER];
static uint8 dispensador_buffer_Lectura [LARGO_BUFFER];

uint8 bus_enviarComandoBooleano(uint8 comando, CYBIT valor){
    uint8 comando_ok = pdFALSE;
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = comando;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1] = valor;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2] = PAQUETE_FIN;
    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        comando_ok = COMUNICACION_FALLIDA;
    }
    else{
        vTaskDelay(pdMS_TO_TICKS(20));
        if(!bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
            comando_ok = COMUNICACION_FALLIDA;
        }
        else{
            if((dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO)&&
            (dispensador_buffer_Lectura[PAQUETE_WR_FIN_POS]==PAQUETE_FIN)){
                if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                    comando_ok = COMANDO_CORRECTO;
                }
                else{
                    comando_ok = COMANDO_ERROR;
                }
            }
        }
    }
    return comando_ok;
}

xMedidaDispensador bus_solicitarMedida(uint8 comando){
    xMedidaDispensador comandoMedida;
    comandoMedida.medida = 0.0f;
    uint8 indice = PAQUETE_ESTADO_POS+1;
    char bufferRespuesta[LARGO_BUFFER];
    vTaskDelay(pdMS_TO_TICKS(10));
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = comando;
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
    
    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        comandoMedida.estado = COMUNICACION_FALLIDA;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    if(!bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
        comandoMedida.estado = COMUNICACION_FALLIDA;
    }
    else{
        if(dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO){
            if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                while(dispensador_buffer_Lectura[indice]!=PAQUETE_FIN){
                    bufferRespuesta[indice-(PAQUETE_ESTADO_POS+1)] = dispensador_buffer_Lectura[indice];
                    indice++;
                }
                comandoMedida.medida = atof(bufferRespuesta);
                comandoMedida.estado = COMANDO_CORRECTO;
            }
            else{
                comandoMedida.estado = COMANDO_ERROR;
            }
        }
    }
    return comandoMedida;
}

uint8 bus_verificar(){
    uint8 comando_ok = pdFALSE;
    int timeOut = 0;
    //bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    //bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        comando_ok = COMUNICACION_FALLIDA; 
    }
    else{
        while(timeOut<10){
            vTaskDelay(pdMS_TO_TICKS(20));
            if(bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
                if((dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO)&&
                (dispensador_buffer_Lectura[PAQUETE_WR_FIN_POS]==PAQUETE_FIN)){
                    if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                        comando_ok = COMANDO_CORRECTO;
                        break;
                    }
                    else{
                        if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_FALLA){
                            comando_ok = COMANDO_ERROR;
                            break;
                        }
                    }
                }
            }
            timeOut++;
        }
    }
    return comando_ok;
}

uint8 Dispensador_LED_DEBUG_write(CYBIT valor){
    uint8 comando_ok;
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    if(valor){
        dispensador_buffer_Escritura[PAQUETE_CMD_POS] = ENCENDER_LED_DEBUG;
    }
    else{
        dispensador_buffer_Escritura[PAQUETE_CMD_POS] = APAGAR_LED_DEBUG;
    }
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
    comando_ok = bus_verificar();
    return comando_ok;
}

uint8 Dispensador_definir_brillo(uint8 valor){
    vTaskDelay(pdMS_TO_TICKS(10));
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = DEFINIR_BRILLO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1] = valor;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2] = PAQUETE_FIN;
    
    return bus_verificar();
}

xMedidaDispensador Dispensador_LeerPeso(){
    xMedidaDispensador comandoPeso;
    comandoPeso = bus_solicitarMedida(LEER_PESO);
    return comandoPeso;
}

uint8 Dispensador_definir_escala_peso(float escala){
    uint8 comando_ok = pdFALSE;
    uint8 escalaPeso = 1.0f/escala;
    
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = DEFINIR_ESCALA_PESO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1]=escalaPeso;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2] = PAQUETE_FIN;

    comando_ok = bus_verificar();
    return comando_ok;
}

float Dispensador_leer_escala_peso(){
    uint8 escalaPeso=0;
    float escala;
    vTaskDelay(pdMS_TO_TICKS(10));
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = LEER_ESCALA_PESO;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;

    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        escala = COMUNICACION_FALLIDA;
    }
    else{
        vTaskDelay(pdMS_TO_TICKS(10));
        if(bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
            if((dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO)&&
            (dispensador_buffer_Lectura[PAQUETE_WR_FIN_POS+1]==PAQUETE_FIN)){
                if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                    escalaPeso=dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+1];
                    escala = COMANDO_CORRECTO;
                }
                else{
                    if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_FALLA){
                        escalaPeso = 0;
                        escala = COMANDO_ERROR;
                    }
                }
            }
        }
    }
    escala = 1.0f/escalaPeso;
    return escala;
}

uint8 Dispensador_setCero_peso(){
    uint8 comando_ok = pdFALSE;
    vTaskDelay(pdMS_TO_TICKS(10));
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = PESO_CERO;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;

    comando_ok = bus_verificar();
    return comando_ok;
}

xMedidaDispensador Dispensador_LeerVoltaje(){
    xMedidaDispensador comandoVoltaje;
    comandoVoltaje.medida = 0.0f;
    uint8 indice = PAQUETE_ESTADO_POS+1;
    char bufferRespuesta[LARGO_BUFFER];
    
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = LEER_VOLTAJE;
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
    
    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        comandoVoltaje.estado = COMUNICACION_FALLIDA;
    }
    else{
        vTaskDelay(pdMS_TO_TICKS(20));
        if(!bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
            comandoVoltaje.estado = COMUNICACION_FALLIDA;
        }
        else{
            if(dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO){
                if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                    while(dispensador_buffer_Lectura[indice]!=PAQUETE_FIN){
                        bufferRespuesta[indice-(PAQUETE_ESTADO_POS+1)] = dispensador_buffer_Lectura[indice];
                        indice++;
                    }
                    comandoVoltaje.medida = atof(bufferRespuesta);
                    comandoVoltaje.estado = COMANDO_CORRECTO;
                }
                else{
                    comandoVoltaje.estado = COMANDO_ERROR;
                }
            }
        }
    }
    return comandoVoltaje;
}

xMedidaDispensador Dispensador_LeerTemperatura(uint8 sensor){
    vTaskDelay(pdMS_TO_TICKS(20));
    xMedidaDispensador comandoTemperatura;
    uint8 comando = 0;
    switch(sensor){
        case 1:
            comando = LEER_TEMPERATURA1;
        break;
        case 2:
            comando = LEER_TEMPERATURA2;
        break;
        default:
            comando = LEER_TEMPERATURA_PROM;
        break;
    }
    comandoTemperatura = bus_solicitarMedida(comando);
    return comandoTemperatura;
}

float Dispensador_LeerTemperaturaDefinida(){
    uint16 temp = 0xFFFF;
    float tempDefinida = 0.0f;
    
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = LEER_TEMPERATURA_DEFINIDA;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;

    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        tempDefinida = COMUNICACION_FALLIDA;
    }
    else{
        vTaskDelay(pdMS_TO_TICKS(20));
        if(bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
            if((dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO)&&
            (dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+3]==PAQUETE_FIN)){
                if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                    temp = dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+1]<<8;
                    temp = temp | dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+2];
                    tempDefinida = temp/100.0f;
                }
                else{
                    if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_FALLA){
                        tempDefinida = COMANDO_ERROR;
                    }
                }
            }
        }
    }
    return tempDefinida;
}

xMedidaDispensador Dispensador_LeerHumedad(uint8 sensor){
    vTaskDelay(pdMS_TO_TICKS(20));
    xMedidaDispensador comandoHumedad;
    uint8 comando = 0;
    switch(sensor){
        case 1:
            comando = LEER_HUMEDAD1;
        break;
        case 2:
            comando = LEER_HUMEDAD2;
        break;
        default:
            comando = LEER_HUMEDAD_PROM;
        break;
    }
    comandoHumedad = bus_solicitarMedida(comando);
    return comandoHumedad;
}

xMedidaDispensador Dispensador_LeerDistancia(uint8 sensor){
    vTaskDelay(pdMS_TO_TICKS(20));
    xMedidaDispensador distancia;
    distancia.medida=0.0f;
    distancia.estado=pdFALSE;
    uint8 indice = PAQUETE_ESTADO_POS+1;
    char bufferRespuesta[LARGO_BUFFER];
    vTaskDelay(pdMS_TO_TICKS(10));
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = LEER_DISTANCIA;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1] = sensor;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+3] = PAQUETE_FIN;
    
    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        distancia.estado = COMUNICACION_FALLIDA;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
    if(!bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
        distancia.estado = COMUNICACION_FALLIDA;
    }
    else{
        if(dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO){
            if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                while(dispensador_buffer_Lectura[indice]!=PAQUETE_FIN){
                    bufferRespuesta[indice-(PAQUETE_ESTADO_POS+1)] = dispensador_buffer_Lectura[indice];
                    indice++;
                }
                distancia.medida = atof(bufferRespuesta);
                distancia.estado = COMANDO_CORRECTO;
            }
            else{
                distancia.estado = COMANDO_ERROR;
            }
        }
    }
    return distancia;
}

CYBIT Dispensador_definir_Tiempos(uint16 tiempos[], uint16 tiempoMovimiento, uint16 cicloUtil){
    uint8 indice=0, n = 0;
    vTaskDelay(pdMS_TO_TICKS(10));
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = DEFINIR_TIEMPO_BANDEJAS;
    for(n=0;n<NUMERO_BANDEJAS_MAX;n++){
        indice=2*n+1;
        dispensador_buffer_Escritura[PAQUETE_CMD_POS+(indice)] = tiempos[n] >> 8;
        dispensador_buffer_Escritura[PAQUETE_CMD_POS+(indice+1)] = tiempos[n] & 0xFF;
    }
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+13] = tiempoMovimiento >> 8;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+14] = tiempoMovimiento & 0xFF;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+15] = cicloUtil>> 8;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+16] = cicloUtil & 0xFF;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+17] = PAQUETE_FIN;
    
    return bus_verificar();
}

uint8 Dispensador_dispensar(uint16 producto){
    uint8 comando_ok = pdFALSE;
    uint8 tempBandeja = (producto/100); //encuentre la bandeja correspondiente
    uint8 tempColumnaMotor = producto-(tempBandeja*100)+1; //encuentre la columna correspondiente
    uint8 motor;
    xConfiguracion parametros;
    xConfiguracionBandejas bandejas;
    xQueuePeek(configuracionesMaquina, &parametros, 10);
    xQueuePeek(configuracionesBandejas, &bandejas, 10);
    vTaskDelay(pdMS_TO_TICKS(50));
    if((tempBandeja<=bandejas.configBandejas.numBandejas)&&(tempColumnaMotor<=MAX_PRODUCTOS_BANDEJA)){
        motor = ((tempBandeja-1)*MAX_PRODUCTOS_BANDEJA)+tempColumnaMotor;
        comando_ok = Dispensador_mover_motor(motor);
    }
    else{
        comando_ok = ERROR_PRODUCTO;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
    return comando_ok;
}

uint8 Dispensador_abrirProducto(uint16 producto){
    uint8 comando_ok = pdFALSE;
    uint8 tempBandeja = (producto/100); //encuentre la bandeja correspondiente
    uint8 tempColumnaBloqueo = producto-(tempBandeja*100)+1; //encuentre la columna correspondiente
    uint8 bloqueo;
    xConfiguracion parametros;
    xConfiguracionBandejas bandejas;
    xQueuePeek(configuracionesMaquina, &parametros, 10);
    xQueuePeek(configuracionesBandejas, &bandejas, 10);
    vTaskDelay(pdMS_TO_TICKS(50));
    if((tempBandeja<=bandejas.configBandejas.numBandejas)&&(tempColumnaBloqueo<=MAX_PRODUCTOS_BANDEJA)){
        bloqueo = ((tempBandeja-1)*MAX_PRODUCTOS_BANDEJA)+tempColumnaBloqueo;
        comando_ok = Dispensador_abrir_slot(bloqueo); 
    }
    else{
        comando_ok = ERROR_PRODUCTO;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
    return comando_ok;
}

uint8 Dispensador_mover_motor(uint8 motor){
    uint8 comando_ok = COMANDO_ERROR;
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = MOVER_MOTOR;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1] = motor;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2] = PAQUETE_FIN;
    comando_ok = bus_verificar();
    return comando_ok;
}

uint8 Dispensador_leer_motor(){
    uint8 comando_ok=pdFALSE;
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = LEER_MOTOR;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;

    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        comando_ok = COMUNICACION_FALLIDA;
    }else{
        vTaskDelay(pdMS_TO_TICKS(200));
        if(bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
            if((dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO)&&
            (dispensador_buffer_Lectura[PAQUETE_WR_FIN_POS]==PAQUETE_FIN)){
                switch(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]){
                    case CMD_ESPERA:
                        comando_ok = COMANDO_ESPERA;
                    break;
                    case CMD_CORRECTO:
                        comando_ok = COMANDO_CORRECTO;
                    break;
                    case CMD_FALLA:
                        comando_ok = COMANDO_ERROR;
                    break;
                    case CMD_NO_CAYO:
                        comando_ok = COMANDO_NO_CAYO;
                    break;
                    case CMD_ALERTA:
                        comando_ok = COMANDO_ALERTA;
                    break;
                }
            }
        }        
    }
    vTaskDelay(pdMS_TO_TICKS(20));
    return comando_ok;
}

uint8 Dispensador_abrir_slot(uint8 slot){
    uint8 comando_ok = pdFALSE;
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = ABRIR_SLOT;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1] = slot;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2] = PAQUETE_FIN;

    comando_ok = bus_verificar();
    return comando_ok;
}

uint8 Dispensador_Modo_Prueba(CYBIT valor){
    uint8 comando_ok;
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    if(valor==1){
        dispensador_buffer_Escritura[PAQUETE_CMD_POS] = MODO_PRUEBA;
    }
    else{
        dispensador_buffer_Escritura[PAQUETE_CMD_POS] = MODO_NORMAL;
    }
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
    comando_ok = bus_verificar();
    vTaskDelay(pdMS_TO_TICKS(10));
    return comando_ok;
}

uint8 Dispensador_definir_peso_minimo(uint16 peso){
    uint8 comando_ok;
    uint16 pesoenvio;
    pesoenvio=peso*10;
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = DEFINIR_PESO_MINIMO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1]= pesoenvio >> 8;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2]= pesoenvio & 0xFF;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+3] = PAQUETE_FIN;

    comando_ok = bus_verificar();
    return comando_ok;
}

uint16 Dispensador_leer_peso_minimo(){
    uint16 peso = 0;
    uint16 leerpesominimo = 0.0f;
    
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = LEER_PESO_MINIMO;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;

    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        leerpesominimo = 0;
    }
    else{
        vTaskDelay(pdMS_TO_TICKS(20));
        if(bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
            if((dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO)&&
            (dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+3]==PAQUETE_FIN)){
                if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                    peso = dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+1]<<8;
                    peso = peso | dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+2];
                    leerpesominimo = peso/10.0f;
                }
                else{
                    if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_FALLA){
                       leerpesominimo = COMANDO_ERROR;
                    }
                }
            }
        }
    }
    return leerpesominimo;
    
}

uint8 Dispensador_definir_temperatura(uint16 temperatura){
    uint8 comando_ok;
    
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = DEFINIR_TEMPERATURA;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1]= temperatura >> 8;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2]= temperatura & 0xFF;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+3] = PAQUETE_FIN;

    comando_ok = bus_verificar();
    return comando_ok;
}

uint8 Dispensador_definir_rango_superior(uint16 rango_sup){
    uint8 comando_ok;
    
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = DEFINIR_RANGO_SUPERIOR;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1]= rango_sup >> 8;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2]= rango_sup & 0xFF;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+3] = PAQUETE_FIN;

    comando_ok = bus_verificar();
    return comando_ok;
}

uint8 Dispensador_definir_rango_inferior(uint16 rango_inf){
    uint8 comando_ok;
    
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = DEFINIR_RANGO_INFERIOR;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1]= rango_inf >> 8;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2]= rango_inf & 0xFF;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+3] = PAQUETE_FIN;

    comando_ok = bus_verificar();
    return comando_ok;
}

uint8 Dispensador_Refrigerador_write(CYBIT valor){
    uint8 comando_ok;
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    if(valor){
        dispensador_buffer_Escritura[PAQUETE_CMD_POS] = ENCENDER_REFRIGERACION;
    }
    else{
        dispensador_buffer_Escritura[PAQUETE_CMD_POS] = APAGAR_REFRIGERACION;
    }
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
    
    comando_ok = bus_verificar();
    return comando_ok;
}

uint8 Dispensador_leer_puerta(){
    uint8 comando_ok = pdFALSE;
    vTaskDelay(pdMS_TO_TICKS(10));
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = LEER_ESTADO_PUERTA;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;

    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        comando_ok = COMUNICACION_FALLIDA;
    }
    else{
        vTaskDelay(pdMS_TO_TICKS(10));
        if(bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
            if((dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO)&&
            (dispensador_buffer_Lectura[PAQUETE_WR_FIN_POS+1]==PAQUETE_FIN)){
                if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                    comando_ok=dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+1];
                }
                else{
                    if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_FALLA){
                        comando_ok = COMANDO_ERROR;
                    }
                }
            }
        }
    }
    return comando_ok;
}

uint8 Dispensador_abrir_puerta(CYBIT valor){
    uint8 comando_ok = pdFALSE;
    vTaskDelay(pdMS_TO_TICKS(10));
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    if(valor){
        dispensador_buffer_Escritura[PAQUETE_CMD_POS] = ABRIR_PUERTA;
    }
    else{
        dispensador_buffer_Escritura[PAQUETE_CMD_POS] = CERRAR_PUERTA;
    }
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;
    comando_ok = bus_verificar();
    return comando_ok;
}

uint8 Dispensador_definir_estado(uint8 estado){
    uint8 comando_ok;
    
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = DEFINIR_ESTADO_TARJETA;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+1]= estado;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS+2] = PAQUETE_FIN;

    comando_ok = bus_verificar();
    return comando_ok;
}

uint8 Dispensador_leer_estado(){
    uint8 estado = pdFALSE;
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = LEER_ESTADO_TARJETA;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;

    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        estado = COMUNICACION_FALLIDA;
    }
    else{
        vTaskDelay(pdMS_TO_TICKS(1));
        if(bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
            if((dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO)&&
            (dispensador_buffer_Lectura[PAQUETE_WR_FIN_POS+1]==PAQUETE_FIN)){
                if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                    estado=dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+1];
                }
                else{
                    if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_FALLA){
                        estado = COMANDO_ERROR;
                    }
                }
            }
        }
    }
    return estado;
}

xCanasta Dispensador_leer_canasta(){
    uint8 comando_ok = pdFALSE;
    xCanasta canasta;
    bus_limpiar_bufferEscritura(dispensador_buffer_Escritura,LARGO_BUFFER);
    bus_limpiar_bufferLectura(dispensador_buffer_Lectura,LARGO_BUFFER);
    dispensador_buffer_Escritura[PAQUETE_INICIO_POS] = PAQUETE_INICIO;
    dispensador_buffer_Escritura[PAQUETE_CMD_POS] = LEER_ESTADO_CANASTA;
    dispensador_buffer_Escritura[PAQUETE_WR_FIN_POS] = PAQUETE_FIN;

    if(!bus_escribir_Comando(DIRECCION_DISPENSADOR,dispensador_buffer_Escritura)){
        comando_ok = COMUNICACION_FALLIDA;
    }
    else{
        vTaskDelay(pdMS_TO_TICKS(1));
        if(bus_leer_comando(DIRECCION_DISPENSADOR, dispensador_buffer_Lectura)){
            if((dispensador_buffer_Lectura[PAQUETE_INICIO_POS]==PAQUETE_INICIO)&&
            (dispensador_buffer_Lectura[PAQUETE_WR_FIN_POS+1]==PAQUETE_FIN)){
                if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_CORRECTO){
                    //canasta.canastaBloquedada=dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+1];
                    canasta.canastaCerrada=dispensador_buffer_Lectura[PAQUETE_ESTADO_POS+1];
                    comando_ok = COMANDO_CORRECTO;
                }
                else{
                    if(dispensador_buffer_Lectura[PAQUETE_ESTADO_POS]==CMD_FALLA){
                        comando_ok = COMANDO_ERROR;
                    }
                }
            }
        }
    }
    canasta.comando=comando_ok;
    return canasta;
}

/* [] END OF FILE */
