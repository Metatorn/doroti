/* ==============================================================
 *
 * Copyright EKIA Technologies, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECH.
 *
 * HMI EKIA 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ==============================================================
;FUNCIONES PROPIAS DE LA PANTALLA TACTIL NEXTION
===============================================================*/
#include "FreeRTOS.h"
#include "task.h"

#include "`$INSTANCE_NAME`_UART.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "`$INSTANCE_NAME`.h"
#include "Clock_variable.h"
#include "RTC_RTC.h"

RTC_RTC_TIME_DATE tiempo;
CYBIT reinicio = 0;

void `$INSTANCE_NAME`_Start(){
    `$INSTANCE_NAME``[UART]`Start();
}
/*******************************************************************************
* FUNCIONES PARA RECEPCION DE COMANDOS DESDE LA PANTALLA TACTIL
*******************************************************************************/

bufferEntradaHMI `$INSTANCE_NAME`_rx_comando(){
    bufferEntradaHMI buffer;
    CYBIT numero=pdFALSE;
    char temp=0;
    int indice = 0;
    int indiceNum = 0;
    int timeOut = 0;
    int contadorFinCadena = 0;
    
    for(indice=0; indice<`$INSTANCE_NAME`_LEN_MAX; indice++){ //limpia el buffer de entrada
        buffer.buffer[indice] = 0;
        buffer.objeto = 0;
        buffer.estado = 0;
        buffer.fin = 0;
    }
    for(indice=0; indice<15; indice++){ //limpia el buffer de entrada
        buffer.numero[indice] = 0;
    }
    indice = 0;
    while((!(`$INSTANCE_NAME`_UART_GetRxBufferSize()>1))&&(timeOut < `$INSTANCE_NAME`_TMAX_HMI)){          //espera a que se reciba un FF
        vTaskDelay(pdMS_TO_TICKS(10));
        timeOut++;
    }
    if(timeOut<`$INSTANCE_NAME`_TMAX_HMI){
        //recibe el primer byte, este será el objeto y puede tener varios significados de error, verificación o tipo de dato
        buffer.objeto=`$INSTANCE_NAME`_UART_GetChar();
        vTaskDelay(pdMS_TO_TICKS(2));
        if(buffer.objeto=='%'){
            reinicio = 1;
            buffer.objeto = `$INSTANCE_NAME`_UART_GetChar();
        }
        while((temp!=';')&&(indice<`$INSTANCE_NAME`_LEN_MAX)&&(contadorFinCadena!=3)){
            temp = `$INSTANCE_NAME`_UART_GetChar();
            //verifica si el byte recibido es un 0xFF, de ser así verifique que sea el final de cadena
            if(temp == 0xFF){
                contadorFinCadena++;
            }
            else{
                contadorFinCadena = 0;
                if(temp==';'){
                    buffer.fin = indice;
                }else{
                    if(numero){
                        buffer.numero[indiceNum]=temp;
                        indiceNum++;
                    }
                    else{
                        buffer.buffer[indice]=temp;
                        if(temp=='#'){
                            numero=pdTRUE;
                        }
                        indice++;
                    }
                }
            }
            vTaskDelay(pdMS_TO_TICKS(1));     
        }
        if(contadorFinCadena==3){
            buffer.estado = `$INSTANCE_NAME`_CORRECTO ;
        }
        else{
            buffer.estado = `$INSTANCE_NAME`_FALLIDO;
        }
        `$INSTANCE_NAME`_UART_ClearRxBuffer();
    }
    return buffer;
}

/*******************************************************************************
* FUNCIONES PARA ENVIO DE COMANDOS A PANTALLA TACTIL
*******************************************************************************/

/*******************************************************************************
* `$INSTANCE_NAME`_tx
********************************************************************************
* Función que envia toda la cadena de caracteres de los comandos generados
* y los 0xFF de confirmación a la pantalla tactil
*******************************************************************************/
void `$INSTANCE_NAME`_tx(char* comando){
    taskENTER_CRITICAL();
    `$INSTANCE_NAME`_UART_PutString(comando);
    `$INSTANCE_NAME`_UART_PutChar(0xFF);
    `$INSTANCE_NAME`_UART_PutChar(0xFF);
    `$INSTANCE_NAME`_UART_PutChar(0xFF);
    taskEXIT_CRITICAL();
    vTaskDelay(pdMS_TO_TICKS(20));
    return;
}


/*******************************************************************************
* `$INSTANCE_NAME`_tx_comando
********************************************************************************
* Función que genera la cadena de caracteres de un comando de la pantalla tactil
*******************************************************************************/
void `$INSTANCE_NAME`_tx_comando(char* objeto, char* numero, char* parametro, char* valor){
    char comando[80];
    stpcpy(comando,objeto);
    strcat(comando,numero);
    strcat(comando,".");
    strcat(comando,parametro);
    strcat(comando,"=");
    if(!strcmp(parametro,"txt")){
        strcat(comando,"\"");
        strcat(comando,valor);
        strcat(comando,"\"");
    }
    else{
        strcat(comando,valor);
    }
    `$INSTANCE_NAME`_tx(comando);
    return;
}

/*******************************************************************************
* `$INSTANCE_NAME`_tx_ventana
********************************************************************************
* Función que permite cambiar entre ventanas de la pantalla
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_ventana(uint8 valor){
    CYBIT comando_Ok = pdFALSE;
    char comando[50];
    char ventana[5];
    int numPagina = 0;
    bufferEntradaHMI buffer;
    strcpy(comando,cmd_pantalla);
    sprintf(ventana,"%d",valor);
    strcat(comando,ventana);
    `$INSTANCE_NAME`_tx(comando);
    vTaskDelay(pdMS_TO_TICKS(150));
    buffer = `$INSTANCE_NAME`_rx_comando();
    sscanf(buffer.buffer,"%d",&numPagina);
    if((buffer.objeto=='P')&&(numPagina==valor)){
        comando_Ok=pdTRUE;
    }
    return comando_Ok;
}

/*******************************************************************************
* tx_brillo
********************************************************************************
* Función que permite cambiar el brillo dinamicamente
*******************************************************************************/
void `$INSTANCE_NAME`_brillo(uint8 valor_brillo){
    char comando[50];
    char brillo[3];
    strcpy(comando,cmd_dim);
    sprintf(brillo,"%d",valor_brillo);
    strcat(comando,brillo);
    `$INSTANCE_NAME`_tx(comando);
}

/*******************************************************************************
* tx_dibujar
********************************************************************************
* Función que permite cambiar el brillo dinamicamente
*******************************************************************************/
void `$INSTANCE_NAME`_dibujo(uint8 habilitar, char*color){
    char comando[30];
    char enable[3];
    strcpy(comando,cmd_draw);
    sprintf(enable,"%d",habilitar);
    strcat(comando,enable);
    `$INSTANCE_NAME`_tx(comando);
    strcpy(comando,cmd_draw_color);
    strcat(comando,color);
    `$INSTANCE_NAME`_tx(comando);
}


/*******************************************************************************
* `$INSTANCE_NAME`_visualizar
********************************************************************************
* Función que permite ocultar o mostrar un objeto de la pantalla
*******************************************************************************/
void `$INSTANCE_NAME`_visualizar(char* objeto, CYBIT visible){
    char comando[50];
    char value[3];
    sprintf(value,"%d",visible);
    strcpy(comando,cmd_vis);
    strcat(comando,objeto);
    strcat(comando,",");
    strcat(comando,value);
    `$INSTANCE_NAME`_tx(comando);
}


/*******************************************************************************
* `$INSTANCE_NAME`_touch
********************************************************************************
* Función que permite activar o desactivar el touch de un objeto de la pantalla
*******************************************************************************/
void `$INSTANCE_NAME`_touch(char* objeto, char* numero, CYBIT habilitado){
    char comando[50];
    char value[3];
    sprintf(value,"%d",habilitado);
    strcpy(comando,cmd_tsw);
    strcat(comando,objeto);
    strcat(comando,numero);
    strcat(comando,",");
    strcat(comando,value);
    `$INSTANCE_NAME`_tx(comando);
}


/*******************************************************************************
* `$INSTANCE_NAME`_print_value
********************************************************************************
* Función que permite obtener valores de objetos de la pantalla sin formato 
*******************************************************************************/
void `$INSTANCE_NAME`_print_value(char* objeto){
    char comando[50];
    
    strcpy(comando,cmd_print_val);
    strcat(comando,"\"T\"");
    `$INSTANCE_NAME`_tx(comando);

    strcpy(comando,cmd_print_val);
    strcat(comando,objeto);
    `$INSTANCE_NAME`_tx(comando);
    
    strcpy(comando,cmd_print_val);
    strcat(comando,"\";\"");
    `$INSTANCE_NAME`_tx(comando);
    vTaskDelay(pdMS_TO_TICKS(10));
}


/*******************************************************************************
* `$INSTANCE_NAME`_get_value
********************************************************************************
* Función que permite obtener valores de objetos de la pantalla con cabecera y
* fin de trama, la cabecera tiene diferentes valores para indicar el tipo de variable
* y fin de trama denotado con 3 bytes 0xFF consecutivos
*******************************************************************************/
void `$INSTANCE_NAME`_get_value(char* objeto){
    char comando[50];
    strcpy(comando,cmd_get);
    strcat(comando,objeto);
    `$INSTANCE_NAME`_tx(comando);
    vTaskDelay(pdMS_TO_TICKS(10));
}


/*******************************************************************************
* `$INSTANCE_NAME`_set_value
********************************************************************************
* Función que permite asignar valores en objetos y variables de la pantalla
*******************************************************************************/
void `$INSTANCE_NAME`_set_value(char* objeto,int valor){
    char comando[50];
    sprintf(comando,"%s%s%d",objeto,cmd_val,valor);
    `$INSTANCE_NAME`_tx(comando);
}

void `$INSTANCE_NAME`_set_float(char* objeto,int valor, int decimales){
    char comando[50];
    sprintf(comando,"%s%s%d",objeto,cmd_val,valor);
    `$INSTANCE_NAME`_tx(comando);
    vTaskDelay(pdMS_TO_TICKS(10));
    sprintf(comando,"%s%s%d",objeto,cmd_decimal,decimales);
    `$INSTANCE_NAME`_tx(comando);
}

/*******************************************************************************
* `$INSTANCE_NAME`_read_time
********************************************************************************
* Función que obtiene la fecha de la pantalla
*******************************************************************************/
RTC_RTC_TIME_DATE *`$INSTANCE_NAME`_read_time(){
    bufferEntradaHMI respuesta;
    uint8 checksum = 0;
    int check = 0, timeOut = 0;
    CYBIT fin = pdFALSE;
    while(!fin){
        do{
            `$INSTANCE_NAME`_get_value("rtc0");
            respuesta = `$INSTANCE_NAME`_rx_comando();
        }while((respuesta.objeto!=`$INSTANCE_NAME`_DATO_NUMERICO)||(respuesta.estado!=`$INSTANCE_NAME`_CORRECTO ));
        tiempo.Year = (respuesta.buffer[1]<<8)|respuesta.buffer[0];
        check += tiempo.Year;
        vTaskDelay(pdMS_TO_TICKS(10));
        `$INSTANCE_NAME`_rx_comando();
        do{
            `$INSTANCE_NAME`_get_value("rtc1");
            respuesta = `$INSTANCE_NAME`_rx_comando(); 
        }while((respuesta.objeto!=`$INSTANCE_NAME`_DATO_NUMERICO)||(respuesta.estado!=`$INSTANCE_NAME`_CORRECTO ));
        tiempo.Month = respuesta.buffer[0];
        check += tiempo.Month;
        vTaskDelay(pdMS_TO_TICKS(10));
        `$INSTANCE_NAME`_rx_comando();
        do{
            `$INSTANCE_NAME`_get_value("rtc2");
            respuesta = `$INSTANCE_NAME`_rx_comando(); 
        }while((respuesta.objeto!=`$INSTANCE_NAME`_DATO_NUMERICO)||(respuesta.estado!=`$INSTANCE_NAME`_CORRECTO ));
        tiempo.DayOfMonth = respuesta.buffer[0];
        check += tiempo.DayOfMonth;
        vTaskDelay(pdMS_TO_TICKS(10));
        `$INSTANCE_NAME`_rx_comando();
        do{
            `$INSTANCE_NAME`_get_value("rtc3");
            respuesta = `$INSTANCE_NAME`_rx_comando();
        }while((respuesta.objeto!=`$INSTANCE_NAME`_DATO_NUMERICO)||(respuesta.estado!=`$INSTANCE_NAME`_CORRECTO ));
        tiempo.Hour = respuesta.buffer[0];
        check += tiempo.Hour;
        vTaskDelay(pdMS_TO_TICKS(10));
        `$INSTANCE_NAME`_rx_comando();
        do{
            `$INSTANCE_NAME`_get_value("rtc4");
            respuesta = `$INSTANCE_NAME`_rx_comando();
        }while((respuesta.objeto!=`$INSTANCE_NAME`_DATO_NUMERICO)||(respuesta.estado!=`$INSTANCE_NAME`_CORRECTO ));
        tiempo.Min = respuesta.buffer[0];
        check += tiempo.Min;
        vTaskDelay(pdMS_TO_TICKS(10));
        `$INSTANCE_NAME`_rx_comando();
        
        `$INSTANCE_NAME`_get_value("Bienvenida.checksum.val");
        respuesta = HMI_rx_comando();
        if((respuesta.objeto==`$INSTANCE_NAME`_DATO_NUMERICO)&&(respuesta.estado==`$INSTANCE_NAME`_CORRECTO )){
            checksum = respuesta.buffer[0];
            vTaskDelay(pdMS_TO_TICKS(10));
            `$INSTANCE_NAME`_rx_comando();
            if((check & 0xFF) == checksum){
                fin = pdTRUE;
            }
        }
        if(timeOut >= 3){
            fin = pdTRUE;
        }
        timeOut++;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return &tiempo;
}


/*******************************************************************************
* `$INSTANCE_NAME`_write_time
********************************************************************************
* Función que escribe la fecha de la pantalla
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_write_time(){
    CYBIT comando_ok = pdFALSE;
    RTC_RTC_TIME_DATE timeDate;
    timeDate = *RTC_RTC_ReadTime();
    char tempValor[20];
    sprintf(tempValor,"rtc0=%d",timeDate.Year);
    `$INSTANCE_NAME`_tx(tempValor);
    sprintf(tempValor,"rtc1=%d",timeDate.Month);
    `$INSTANCE_NAME`_tx(tempValor);
    sprintf(tempValor,"rtc2=%d",timeDate.DayOfMonth);
    `$INSTANCE_NAME`_tx(tempValor);
    sprintf(tempValor,"rtc3=%d",timeDate.Hour);
    `$INSTANCE_NAME`_tx(tempValor);
    sprintf(tempValor,"rtc4=%d",timeDate.Min);
    `$INSTANCE_NAME`_tx(tempValor);
    //sprintf(tempValor,"rtc5=%d",timeDate.Sec);
    //`$INSTANCE_NAME`_tx(tempValor);
    return comando_ok;
}


/*******************************************************************************
* `$INSTANCE_NAME`_calibrar
********************************************************************************
* Función que permite recalibrar el panel tactil de la pantalla
*******************************************************************************/
void `$INSTANCE_NAME`_calibrar(){
    char comando[30];
    strcpy(comando,cmd_calibrar);
    `$INSTANCE_NAME`_tx(comando);
}


/*******************************************************************************
* `$INSTANCE_NAME`_obtenerPagina
********************************************************************************
* Función que permite saber en qué pagina se encuentra actualmente
*******************************************************************************/
int `$INSTANCE_NAME`_obtenerPagina(){
    char comando[50];
    int numPagina = 0;
    bufferEntradaHMI entrada;
    strcpy(comando,cmd_getPagina);
    `$INSTANCE_NAME`_tx(comando);
    vTaskDelay(pdMS_TO_TICKS(5));
    entrada = `$INSTANCE_NAME`_rx_comando();
    if(entrada.objeto==0x66){
        numPagina = entrada.buffer[0];
    }
    return numPagina;
}


/*******************************************************************************
* `$INSTANCE_NAME`_tx_setBaud
********************************************************************************
* Función que envia toda la cadena de caracteres de los comandos generados
* y los 0xFF de confirmación a la pantalla tactil
*******************************************************************************/
CYBIT `$INSTANCE_NAME`_setBaud_default(int baudios){
    CYBIT comando_Ok = pdTRUE;
    char comando[30];
    char tasa[6];
    
    strcpy(comando,cmd_bauds);
    sprintf(tasa,"%d",baudios);
    strcat(comando,tasa);
    `$INSTANCE_NAME`_tx(comando);
    switch(baudios){
        case 9600:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_9600,0);
        break;
        case 19200:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_19200,0);
        break; 
        case 38400:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_38400,0);
        break;  
        case 57600:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_57600,0);
        break;
        case 115200:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_115200,0);
        break; 
        default:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_9600,0);
            comando_Ok = pdFALSE;
        break;    
    }
    return comando_Ok;
}

CYBIT `$INSTANCE_NAME`_setBaud(int baudios){
    CYBIT comando_Ok = pdTRUE;
    char comando[30];
    char tasa[6];
    
    strcpy(comando,cmd_baud);
    sprintf(tasa,"%d",baudios);
    strcat(comando,tasa);
    `$INSTANCE_NAME`_tx(comando);
    switch(baudios){
        case 9600:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_9600,0);
        break;
        case 19200:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_19200,0);
        break; 
        case 38400:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_38400,0);
        break;  
        case 57600:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_57600,0);
        break;
        case 115200:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_115200,0);
        break; 
        default:
            Clock_variable_SetDividerRegister(`$INSTANCE_NAME`_BAUD_9600,0);
            comando_Ok = pdFALSE;
        break;    
    }
    return comando_Ok;
}

CYBIT `$INSTANCE_NAME`_leerReinicio(){
    CYBIT reset = reinicio;
    reinicio = 0;
    return reset;
}

/*******************************************************************************
* tx_Selector_texto
********************************************************************************
* Función que permite cambiar el brillo dinamicamente
*******************************************************************************/
void `$INSTANCE_NAME`_combo_texto(CYBIT lineaInicial, char* objeto, char* texto){
    char comando[50];
    strcpy(comando,objeto);
    strcat(comando,cmd_comboBox);
    if(lineaInicial==pdTRUE){
        strcat(comando,"=\"");
    }
    else{
        strcat(comando,"+=\"\\r");
    }
    strcat(comando,texto );
    strcat(comando,"\"");
    `$INSTANCE_NAME`_tx(comando);
}

/* [] END OF FILE */

