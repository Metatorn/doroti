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

#include "lectorHuella.h"
#include "FINGER.h"
#include "colas.h"
#include "tipos.h"
#include "semaforos.h"

#include "LED_DEBUG.h"

uint8 bufferRecepcion[FINGER_LONGITUD_BUFFER];
int com = 0;

/*******************************************************************************
* limpiarBuffer
********************************************************************************
*
* Tarea que realiza:
*  Funcion que limpia el buffer de recepción temporal
*
*******************************************************************************/
static void finger_limpiarBuffer(){
    uint8 i;
    for(i=0; i<(sizeof bufferRecepcion); i++){
        bufferRecepcion[i]=0;
    } 
}

/*******************************************************************************
* limpiarTodo
********************************************************************************
*
* Tarea que realiza:
*  Funcion que limpia todos los buffer involucrados en el manejo del Lector
*
*******************************************************************************/
void finger_limpiarTodo(){
    FINGER_ClearRxBuffer();
    finger_limpiarBuffer();
}

/*******************************************************************************
* habilitacion
********************************************************************************
*
* Tarea que realiza:
*  Funcion que inicializa los componentes y perifericos relacionados con el lector
*
*******************************************************************************/
void finger_habilitacion(){
    lectorOcupado = xSemaphoreCreateMutex();
    FINGER_Start();
    finger_limpiarBuffer();
}

/*******************************************************************************
* recibirRespuesta
********************************************************************************
*
* Tarea que realiza:
*  Funcion que copia el buffer del puerto serie al buffer temporal para obtener
*  los comandos y respuestas que hayan sido recibidos
*
*******************************************************************************/

CYBIT finger_recibirRespuesta(){
    CYBIT comando_ok = pdFALSE;
    uint8 i = 0;
    int timeOut=0;
    vTaskDelay(pdMS_TO_TICKS(100));
    while((!(FINGER_GetRxBufferSize()>1))&&(timeOut < FINGER_TIMEOUT)){        
        vTaskDelay(pdMS_TO_TICKS(50));
        timeOut++;
    }
    if(timeOut<FINGER_TIMEOUT){
        for(i=0;i<FINGER_LONGITUD_BUFFER;i++){
            //bufferRecepcion[i]=FINGER_GetChar();
            bufferRecepcion[i]=FINGER_rxBuffer[i];
            vTaskDelay(pdMS_TO_TICKS(10));  
        }
        if((bufferRecepcion[0]==FINGER_INICIO1)&&(bufferRecepcion[1]==FINGER_INICIO2)){
            comando_ok = pdTRUE;
        }  
    }
    //FINGER_ClearRxBuffer();
    /*depuracion.dato1 = bufferRecepcion[8]; //ack
    depuracion.dato2 = bufferRecepcion[4];
    depuracion.dato3 = bufferRecepcion[5];
    depuracion.dato4 = com;
    strcpy(depuracion.dato8,"si");
    xQueueSend(cadena2,(void*)&depuracion,10);*/
    return comando_ok;
}

/*******************************************************************************
* calcularChecksum
********************************************************************************
*
* Tarea que realiza:
*  Calcula el checksum de un arreglo determinado por la longitud del mismo
*
*******************************************************************************/
uint16 finger_calcularChecksum(uint8* cadena, uint8 longitud){
    uint16 checksum = 0;
    uint8 i = 0;
    for(i=0;i<longitud;i++){
        checksum += cadena[i];
    }
    return checksum;
}

/*******************************************************************************
* enviarComando
********************************************************************************
*
* Tarea que realiza:
*  Funcion que genera la trama de comandos para enviar solicitudes al lector de huellas
*
*******************************************************************************/
void finger_enviarComando(uint16 comando, uint32 parametro){
    uint8 tramaSalida[FINGER_LONGITUD_BUFFER];
    uint16 checksum=0;
    
    finger_limpiarTodo();
    tramaSalida[0] = FINGER_INICIO1;
    tramaSalida[1] = FINGER_INICIO2;
    tramaSalida[2] = FINGER_DISPOSITIVO_ID1;
    tramaSalida[3] = FINGER_DISPOSITIVO_ID2;
    tramaSalida[4] = (parametro & 0x000000FF);
    tramaSalida[5] = (parametro & 0x0000FF00) >> 8; 
    tramaSalida[6] = (parametro & 0x00FF0000) >> 16; 
    tramaSalida[7] = (parametro & 0xFF000000) >> 24;
    tramaSalida[8] = (comando & 0x00FF);
    tramaSalida[9] = (comando & 0xFF00) >> 8; 
    checksum = finger_calcularChecksum(tramaSalida,10);
    tramaSalida[10] = (checksum & 0x00FF);
    tramaSalida[11] = (checksum & 0xFF00) >> 8; 
    FINGER_PutArray(tramaSalida,FINGER_LONGITUD_BUFFER);
    return;
}

/*******************************************************************************
* verificar
********************************************************************************
*
* Tarea que realiza:
*   Funcion que verifica el buffer y extrae los comandos necesarios, retorna:
*   0x00 => Error de comunicación
*   0xFF => Error de Checksum
*   parametro => Comando ejecutado correctamente
*   codigo error => ocurrio un error que es debidamente identificado
*
*******************************************************************************/
xRespuestaLector finger_verificarComando(){
    xRespuestaLector respuestaLector;
    uint16 respuesta = 0;
    uint32 parametro = 0;
    uint16 checksum = 0, check = 0;
    xcadenas depuracion;
    
    respuestaLector.ack = pdFALSE;
    respuestaLector.codigo = 0x00;
    if((bufferRecepcion[2]==FINGER_DISPOSITIVO_ID1)&&(bufferRecepcion[3]==FINGER_DISPOSITIVO_ID2)){
        checksum = finger_calcularChecksum(bufferRecepcion,10);
        check = (bufferRecepcion[11]<<8)|(bufferRecepcion[10]);
        if(check == checksum){
            respuesta = (bufferRecepcion[9]<<8)|bufferRecepcion[8];
            parametro = bufferRecepcion[4];
            parametro = parametro|(bufferRecepcion[5]<<8);
            parametro = parametro|(bufferRecepcion[6]<<16); 
            parametro = parametro|(bufferRecepcion[7]<<24);
            respuestaLector.codigo = parametro;
            if(respuesta == FINGER_ACK){
                respuestaLector.ack = pdTRUE;
            }
        }
        else{
            respuestaLector.codigo = 0xFF;
        }
    }
    depuracion.dato1 = respuesta;
    depuracion.dato2 = parametro;
    depuracion.dato3 = respuestaLector.codigo;
    depuracion.dato4 = com;
    strcpy(depuracion.dato8,"si");
    xQueueSend(cadena2,(void*)&depuracion,10);
    finger_limpiarTodo();
    return respuestaLector;
}

/*******************************************************************************
* open
********************************************************************************
*
* Tarea que realiza:
*  Funcion que enciende o apaga el LED de la placa del lector de Huellas
*
*******************************************************************************/
CYBIT finger_open(){
    CYBIT comando_ok = pdFALSE;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_OPEN,0);
    if(finger_recibirRespuesta()){
        respuesta = finger_verificarComando();
        if(respuesta.ack){
            comando_ok = pdTRUE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* close
********************************************************************************
*
* Tarea que realiza:
*  Finaliza el dispositivo, en teoria no hace nada
*
*******************************************************************************/
CYBIT finger_close(){
    CYBIT comando_ok = pdFALSE;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_CLOSE,0);
    if(finger_recibirRespuesta()){
        respuesta = finger_verificarComando();
        if(respuesta.ack){
            comando_ok = pdTRUE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* setLED
********************************************************************************
*
* Tarea que realiza:
*  Funcion que enciende o apaga el LED de la placa del lector de Huellas
*
*******************************************************************************/
CYBIT finger_SetLED(CYBIT estado){
    CYBIT comando_ok = pdFALSE;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_LED,estado);
    if(finger_recibirRespuesta()){
        respuesta = finger_verificarComando();
        if(respuesta.ack){
            comando_ok = pdTRUE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* cambiarBaudios
********************************************************************************
*
* Tarea que realiza:
*  Funcion que cambia la tasa de baudios
*
*******************************************************************************/
CYBIT finger_cambiarBaudios(int baudios){
    CYBIT comando_ok = pdFALSE;
    xRespuestaLector respuesta;
    if ((baudios == 9600) || (baudios == 19200) || (baudios == 38400) || (baudios == 57600) || (baudios == 115200)){
        finger_enviarComando(FINGER_CMD_BAUDRATE,baudios);
        if(finger_recibirRespuesta()){
            respuesta = finger_verificarComando();
            if(respuesta.ack){
                comando_ok = pdTRUE;
            }
        }
    }
    return comando_ok;
}

/*******************************************************************************
* totalHuellas
********************************************************************************
*
* Tarea que realiza:
*  Funcion que retorna el total de huellas que hayan sido almacenadas
*
*******************************************************************************/
xRespuestaLector finger_totalHuellas(){
    xRespuestaLector respuesta;
    respuesta.ack = 0;
    finger_enviarComando(FINGER_CMD_ENROLLCOUNT,0);
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        respuesta = finger_verificarComando();
    }
    return respuesta;
}

/*******************************************************************************
* verificarInscripcion
********************************************************************************
*
* Tarea que realiza:
*  Funcion que indica si ya hay una huella en un id determinado
*
*******************************************************************************/
CYBIT finger_verificarInscripcion(uint32 ID){
    CYBIT comando_ok = pdFALSE;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_ENROLLED,ID);
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        respuesta = finger_verificarComando();
        if(respuesta.ack){
            comando_ok = pdTRUE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* iniciarInscribir
********************************************************************************
*
* Tarea que realiza:
*  Funcion que inicia el proceso de inscripción
*
*******************************************************************************/
xRespuestaLector finger_iniciarInscripcion(uint32 ID){
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_ENROLLSTART,ID);
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        respuesta = finger_verificarComando();
    }
    return respuesta;
}

/*******************************************************************************
* incribirPlantilla1
********************************************************************************
*
* Tarea que realiza:
*  Funcion que obtiene el primer escaneado de una plantilla
*
*******************************************************************************/
xRespuestaLector finger_inscribirPlantilla1(){
    com=1;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_ENROLL1,0);
    //vTaskDelay(pdMS_TO_TICKS(100));
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        respuesta = finger_verificarComando();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    com=0;
    return respuesta;
}

/*******************************************************************************
* incribirPlantilla2
********************************************************************************
*
* Tarea que realiza:
*  Funcion que obtiene el segundo escaneado de una plantilla
*
*******************************************************************************/
xRespuestaLector finger_inscribirPlantilla2(){
    com=2;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_ENROLL2,0);
    //vTaskDelay(pdMS_TO_TICKS(100));
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        respuesta = finger_verificarComando();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    com=0;
    return respuesta;
}

/*******************************************************************************
* incribirPlantilla3
********************************************************************************
*
* Tarea que realiza:
*  Funcion que obtiene el tercer escaneado de una plantilla, finaliza la inscripcion
*
*******************************************************************************/
xRespuestaLector finger_inscribirPlantilla3(){
    com=3;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_ENROLL3,0);
    vTaskDelay(pdMS_TO_TICKS(100));
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        respuesta = finger_verificarComando();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    com=0;
    return respuesta;
}

/*******************************************************************************
* dedoPresionado
********************************************************************************
*
* Tarea que realiza:
*  Funcion que indica si hay un dedo en el lector
*
*******************************************************************************/
CYBIT finger_dedoPresionado(){
    CYBIT comando_ok = pdFALSE;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_ISFINGER,0);
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        respuesta = finger_verificarComando();
        if(respuesta.ack){
            if(respuesta.codigo==0){
                comando_ok = pdTRUE;
            }
        }
    }
    return comando_ok;
}

/*******************************************************************************
* borrarInscripcion
********************************************************************************
*
* Tarea que realiza:
*  Funcion que elimina un identificador
*
*******************************************************************************/
CYBIT finger_borrarInscripcion(uint32 ID){
    CYBIT comando_ok = pdFALSE;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_DELETEID,ID);
    if(finger_recibirRespuesta()){
        respuesta = finger_verificarComando();
        if(respuesta.ack){
            comando_ok = pdTRUE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* borrarTodos
********************************************************************************
*
* Tarea que realiza:
*  Funcion que elimina todos los identificadores
*
*******************************************************************************/
CYBIT finger_borrarTodos(){
    CYBIT comando_ok = pdFALSE;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_DELETEALL,0);
    if(finger_recibirRespuesta()){
        respuesta = finger_verificarComando();
        if(respuesta.ack){
            comando_ok = pdTRUE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* verificarEspecifico
********************************************************************************
*
* Tarea que realiza:
*  Funcion que compara una huella con un identificador específico
*
*******************************************************************************/
xRespuestaLector finger_verificarEspecifico(uint32 ID){
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_VERIFY,ID);
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        respuesta = finger_verificarComando();
    }
    return respuesta;
}

/*******************************************************************************
* identificarHuella
********************************************************************************
*
* Tarea que realiza:
*  Funcion que identifica una huella, retorna el ID en caso de encontrar una
*  coincidencia, de lo contrario retorna un error
*
*******************************************************************************/
xRespuestaLector finger_identificarHuella(){
    com = 4;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_IDENTIFY,0);
    vTaskDelay(pdMS_TO_TICKS(100));
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        //LED_DEBUG_Write(1);
        respuesta = finger_verificarComando();
    }
    return respuesta;
}

/*******************************************************************************
* capturarHuella
********************************************************************************
*
* Tarea que realiza:
*  Funcion que guarda una huella, como valor de entrada acepta 0 para un escaneo
*  rápido o cualquier otro valor para un escaneo de alta calidad
*
*******************************************************************************/
xRespuestaLector finger_capturarHuella(CYBIT altaCalidad){
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_CAPTURE,altaCalidad);
    vTaskDelay(pdMS_TO_TICKS(200));
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        respuesta = finger_verificarComando();
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    return respuesta;
}

/*******************************************************************************
* definirSeguridad
********************************************************************************
*
* Tarea que realiza:
*  Define el nivel de seguridad del lector de huellas, valor comprendido entre 1 y 5
*
*******************************************************************************/
CYBIT finger_definirSeguridad(uint8 nivelSeguridad){
    CYBIT comando_ok = pdFALSE;
    if((nivelSeguridad>=1)&&(nivelSeguridad<=5)){
        finger_enviarComando(FINGER_CMD_SETSECURITY,nivelSeguridad);
        if(finger_recibirRespuesta()){
            if(finger_verificarComando().ack){
                comando_ok = pdTRUE;
            }
        }
    }
    return comando_ok;
}

/*******************************************************************************
* obtenerSeguridad
********************************************************************************
*
* Tarea que realiza:
*  obtiene el nivel de seguridad del lector de huellas
*
*******************************************************************************/
xRespuestaLector finger_obtenerSeguridad(){
    xRespuestaLector respuesta;
    respuesta.ack = 0;
    finger_enviarComando(FINGER_CMD_GETSECURITY,0);
    respuesta.ack = finger_recibirRespuesta();
    if(respuesta.ack){
        respuesta = finger_verificarComando();
    }
    return respuesta;
}

/*******************************************************************************
* entrarDescanso
********************************************************************************
*
* Tarea que realiza:
*  Funcion que entra en el modo descanso para ahorro de energía
*
*******************************************************************************/
CYBIT finger_entrarDescanso(){
    CYBIT comando_ok = pdFALSE;
    xRespuestaLector respuesta;
    finger_enviarComando(FINGER_CMD_ENTERSTANDBY,0);
    if(finger_recibirRespuesta()){
        respuesta = finger_verificarComando();
        if(respuesta.ack){
            comando_ok = pdTRUE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* salirDescanso
********************************************************************************
*
* Tarea que realiza:
*  Funcion que sale del modo descanso 
*******************************************************************************/
CYBIT finger_salirDescanso(){
    FINGER_PutChar(0x00);
    vTaskDelay(pdMS_TO_TICKS(30));
    return pdTRUE;
}

/*******************************************************************************
* leerHuella
********************************************************************************
*
* Tarea que realiza:
*  Funcion que retorna un ID identificado al leer una huella puesta
*******************************************************************************/
int finger_leerHuella(){
    int ID = -2;
    xRespuestaLector respuesta;
    if(finger_dedoPresionado()){
        respuesta = finger_capturarHuella(pdFALSE);
		if(respuesta.ack){
            vTaskDelay(pdMS_TO_TICKS(100));
            respuesta.ack = pdFALSE;
            respuesta = finger_identificarHuella();
            ID = -1;
            if(respuesta.ack==pdTRUE){
                ID = respuesta.codigo;
            }
        }
    }
    return ID;
}

/*******************************************************************************
* registrarHuella
********************************************************************************
*
* Tarea que realiza:
*  Funcion que registra una huella en un ID no utilizado
*******************************************************************************/
xRespuestaLector finger_registroHuella(uint8 paso, uint8 indice, uint8 offset){
    xRespuestaLector respuesta;
    respuesta.ack = pdFALSE;
    respuesta.codigo = 0;
    
    switch(paso){
        case 1:{//busca un identificador libre
            indice=1+offset;
            while(finger_verificarInscripcion(indice)==pdTRUE){
                indice++;
            }
            paso++;
        break;}
        case 2:{//inicia el proceso de inscripción
            respuesta = finger_iniciarInscripcion(indice);
            if(respuesta.ack==pdTRUE){
                //si está disponible la inscripción continue, enciende el LED
                finger_SetLED(pdTRUE);
                paso++;
            }
            else{
                paso=0;
            }
        break;}
        case 3:{//espere a que haya un dedo en el sensor
            if(finger_dedoPresionado()==pdFALSE){
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            else{   
                //capture la huella
                respuesta = finger_capturarHuella(pdTRUE);
                if(respuesta.ack==pdTRUE){
                    paso++;
                }
                    else{
                    paso=0;
                }
            }
        break;}
        case 4:{//si todo va bien registre la primera plantilla
            respuesta = finger_inscribirPlantilla1();
            if(respuesta.ack==pdTRUE){
                paso++;
            }
            else{
                paso=0;
            }
        break;}
        case 5:{//espere a que se retire el dedo del sensor
            if(finger_dedoPresionado()==pdTRUE){
                finger_SetLED(pdFALSE);
                vTaskDelay(pdMS_TO_TICKS(100));
                finger_SetLED(pdTRUE);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            else{
                finger_SetLED(pdTRUE);
                paso++;
            }
        break;}
        case 6:{//espere a que haya un dedo en el sensor
            if(finger_dedoPresionado()==pdFALSE){
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            else{
                respuesta = finger_capturarHuella(pdTRUE);
                if(respuesta.ack==pdTRUE){
                    paso++;
                }
                else{
                    paso=0;
                }
            }
        break;}
        case 7:{
            respuesta = finger_inscribirPlantilla2();
            if(respuesta.ack==pdTRUE){
                paso++;
            }
            else{
                paso=0;
            }
        break;}
        case 8:{
            if(finger_dedoPresionado()==pdTRUE){
                finger_SetLED(pdFALSE);
                vTaskDelay(pdMS_TO_TICKS(100));
                finger_SetLED(pdTRUE);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            else{
                finger_SetLED(pdTRUE);
                paso++;
            }
        break;}
        case 9:{
            if(finger_dedoPresionado()==pdFALSE){
                vTaskDelay(pdMS_TO_TICKS(100));
            }
            else{
                respuesta = finger_capturarHuella(pdTRUE);
                if(respuesta.ack==pdTRUE){
                    paso++;
                }
                else{
                    paso=0;
                }
            }
        break;}
        case 10:{
            respuesta = finger_inscribirPlantilla3();
            if(respuesta.ack==pdTRUE){
                paso++;
            }
            else{
                paso=0;
            }
        break;}
        case 11:{
            finger_SetLED(pdFALSE);
            paso++;
        break;}
        default:{
            finger_SetLED(pdFALSE);
            respuesta.ack = pdFALSE;
            respuesta.codigo = 0;
        break;}
    }
    respuesta.id = indice;
    respuesta.aux = paso;
    return respuesta;
}
/* [] END OF FILE */
