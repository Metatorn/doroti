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
 *
 * ===================================================
;FUNCIONES PROPIAS DE LA MEMORIA EEPROM INTERNA
===============================================================*/
#include "EKIA_CONFIG.h"
#include "FreeRTOS.h"
#include "task.h"
#include "colas.h"
#include "tipos.h"
#include "semaforos.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

#include "memEEPROM.h"
#include "EEPROM.h"
//#include "LED_DEBUG.h"


/*******************************************************************************
* habilitarEEPROM
********************************************************************************
* Función para habilitar la memoria interna
*******************************************************************************/
void habilitarEEPROM(){
    EEPROM_Start();
}


/*******************************************************************************
* deshabilitarEEPROM
********************************************************************************
* Función para deshabilitar la memoria interna
*******************************************************************************/
void deshabilitarEEPROM(){
    EEPROM_Stop();
}


/*******************************************************************************
* leerSerie
********************************************************************************
* Función para obtener el numero de serie de la maquina
*******************************************************************************/
xnumSerie leerSerie(){
    xnumSerie serial;
    uint8 indice;
    char temp;
    for(indice=0; indice<LONGITUD_SERIE;indice++){
        temp = EEPROM_ReadByte(DIRECCION_BASE+indice);
        serial.numSerie[indice]=temp;
    }//DOROTI010919A001
    serial.numSerie[LONGITUD_SERIE]=0;
    //serial.tipo = serial.numSerie[5];
    serial.version[0] = serial.numSerie[6];
    serial.version[1] = serial.numSerie[7];
    serial.version[2] = 0;
    for(indice=0;indice<4;indice++){
        serial.nombre[indice]=serial.numSerie[indice];
    }
    serial.nombre[4]=0;
    for(indice=0;indice<4;indice++){
        serial.fecha[indice]=serial.numSerie[indice+8];
    }
    serial.fecha[4]=0;
    for(indice=0;indice<4;indice++){
        serial.serie[indice]=serial.numSerie[indice+12];
    }
    serial.serie[4]=0;
    return serial;
}


/*******************************************************************************
* escribirSerie
********************************************************************************
* Función para escribir el numero de serie de la máquina.
* la función solo permite ser ejecutada si no hay un numero de serie guardado.
* retorna un valor entero que puede tener los siguientes valores:
* ERROR_ESCRITURA
* SERIE_GUARDADA_CORRECTAMENTE
* SERIE_NO_PUEDE_GUARDARSE
*******************************************************************************/
uint8 escribirSerie(){
    uint8 comando = SERIE_GUARDADA_CORRECTAMENTE;
    xnumSerie serialGuardado;
    uint8 indice = 0;
    //DOROTI011219A001
    char serie[16]; 
    stpcpy(serie,"DOROTI");
    //\strcat(serie,DEPENDENCIA);
    //strcat(serie,TIPO_MAQUINA);
    //strcat(serie,SIZE);
    strcat(serie,VERSION);
    strcat(serie,FECHA);
    strcat(serie,NUMERO_DE_SERIE);
    
    serialGuardado = leerSerie();
    if(serialGuardado.numSerie[0]==0){//verifique si hay un numero de serie ya guardado
        EEPROM_UpdateTemperature();
        for(indice=0;indice<LONGITUD_SERIE;indice++){
            if(EEPROM_WriteByte(serie[indice],DIRECCION_BASE+indice)!=CYRET_SUCCESS){
                comando=ERROR_ESCRITURA;
            }
            escribirEscalaPeso(ESCALA_PESO);
        }
    }
    else{//ya hay un numero de serie, no lo cambie
        comando = SERIE_NO_PUEDE_GUARDARSE;
    }
    return comando;
}  

uint8 actualizarSerie(char* version, char* fecha, char* numero){
    uint8 comando = SERIE_GUARDADA_CORRECTAMENTE;
    uint8 indice = 0;
    
    char serie[16];
    //DOROTI011219A001
    stpcpy(serie,"DOROTI");
    //strcat(serie,dependencia);
    //strcat(serie,tipoMaquina);
    //strcat(serie,size);
    strcat(serie,version);
    strcat(serie,fecha);
    strcat(serie,numero);
    
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_SERIE;indice++){
        if(EEPROM_WriteByte(serie[indice],DIRECCION_BASE+indice)!=CYRET_SUCCESS){
            comando=ERROR_ESCRITURA;
        }
    }
   
    return comando;
}  


/*******************************************************************************
* obtenerDireccion
********************************************************************************
* Función para obtener la dirección de un producto determinado que quiera ser consultado.
*
* cada fila puede tener hasta 6 motores, por lo tanto se multiplica este valor 
* por la fila esto indica el número grueso de registros. se tiene además un registro 
* fino que dependerá de la columna del producto, este registro se multiplica por 
* 4 bytes y será sumado al registro grueso, con ello puede encontrarse la dirección 
* que debe ser leida y que corresponda al producto solicitado.
*******************************************************************************/
int obtenerDireccion(int numeroProducto){//201=161
    int fila = numeroProducto/100;//se obtiene la fila del producto
    int columna = numeroProducto-(fila*100); //se obtiene la columna del producto
    int registroMayor = (fila-1)*(NUMERO_MOTORES_FILA*BYTES_REGISTRO); //se calcula dirección del bloque de 60 motores 
    int registroMenor = (columna)*BYTES_REGISTRO; //se calcula direccion especifica dentro del bloque 0
    int direccion = DIRECCION_BASE_PRODUCTO+(registroMayor+registroMenor);
    return direccion;
}


/*******************************************************************************
* leerProducto
********************************************************************************
* Función para obtener el estado de un producto, incluye precio y estado.
* 
*******************************************************************************/
xProducto leerProducto(int numeroProducto){
    xProducto producto;
    int direccion = obtenerDireccion(numeroProducto);//obtiene la direccion
    int factorEscala;
    uint8 indice, temp;
    uint8 arregloProducto[BYTES_REGISTRO]; 
    uint16 registroProducto;
    
    for(indice=0; indice<BYTES_REGISTRO;indice++){ //lee la EEPROM para obtener la informacion
        temp = EEPROM_ReadByte(direccion+indice);
        arregloProducto[indice]=temp;
    }
    producto.numero = numeroProducto;
    
    registroProducto = (arregloProducto[0]<<8)|(arregloProducto[1]);//concatena los bytes de producto
    
    producto.habilitado = registroProducto&0x0001; //obtiene el estado del producto
    registroProducto = registroProducto>>1;
    
    factorEscala = pow(10,(registroProducto&0x007));//obtiene la potencia de 10 por el cual multiplicar el precio
    registroProducto = registroProducto>>3;
    
    //producto.precio = (registroProducto*factorEscala)/factorDecimal;
    producto.precio = (registroProducto*factorEscala)/FACTOR;
    
    producto.cantidad = arregloProducto[2];
    producto.proveedor = arregloProducto[3];
    producto.alturaXItem = (arregloProducto[4]<<8)|(arregloProducto[5]);
    return producto;
}


/*******************************************************************************
* escribirProducto
********************************************************************************
* Función para escribir el estado de un producto, incluye precio y estado del motor.
* 
*******************************************************************************/
CYBIT escribirProducto(xProducto producto){
    CYBIT comando_ok = pdTRUE;
    uint8 arregloProducto[BYTES_REGISTRO]; 
    uint8 factorEscala;
    uint8 indice=0;
    int precio=producto.precio;
    int direccion;
    direccion = obtenerDireccion(producto.numero);
   
    factorEscala = 2; 
    if(precio!=0){
        while (((precio%10)==0)&&(precio!=0)){ //divida entre 10 hasta que quede un numero no terminado en 0
            precio=precio/10; 
            factorEscala++; //si se puede dividir, aumente en 1 el factor de escala
        }
    }
    
    precio = precio << 4;
    factorEscala = factorEscala << 1;
    
    precio = precio | factorEscala | producto.habilitado;
    arregloProducto[0] = (precio>>8) & 0xFF;
    arregloProducto[1] = precio & 0xFF;
    arregloProducto[2] = producto.cantidad;
    arregloProducto[3] = producto.proveedor;
    arregloProducto[4] = (producto.alturaXItem>>8) & 0xFF;
    arregloProducto[5] = producto.alturaXItem & 0xFF;
    
    EEPROM_UpdateTemperature();
    for(indice=0;indice<BYTES_REGISTRO;indice++){
        if(EEPROM_WriteByte(arregloProducto[indice],direccion+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    
    return comando_ok;
}    


/*******************************************************************************
* leerContraseña
********************************************************************************
* Función para obtener la contraseña de usuario
*******************************************************************************/
xclave EEPROM_leerClaveAdmin(){
    xclave clave;
    uint8 indice;
    char temp;
    for(indice=0; indice<LONGITUD_CLAVE;indice++){
        temp = EEPROM_ReadByte(DIRECCION_CONTRA_ADMIN+indice);
        clave.clave[indice]=temp;
    }
    return clave;
}


/*******************************************************************************
* escribirClave
********************************************************************************
* Función para modificar la contraseña del usuario
*******************************************************************************/
CYBIT EEPROM_escribirClaveAdmin(xclave clave){
    CYBIT comando_ok=pdTRUE;
    uint8 indice = 0;
    
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_CLAVE;indice++){
        if(EEPROM_WriteByte(clave.clave[indice],DIRECCION_CONTRA_ADMIN+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}  

/*******************************************************************************
* leerContraseñaTesorero
********************************************************************************
* Función para obtener la contraseña de tesorero
*******************************************************************************/
xclave EEPROM_leerClaveTesorero(){
    xclave clave;
    uint8 indice;
    char temp;
    for(indice=0; indice<LONGITUD_CLAVE;indice++){
        temp = EEPROM_ReadByte(DIRECCION_CONTRA_TESOR+indice);
        clave.clave[indice]=temp;
    }
    return clave;
}

/*******************************************************************************
* escribirClaveTesorero
********************************************************************************
* Función para modificar la contraseña del tesorero
*******************************************************************************/
CYBIT EEPROM_escribirClaveTesorero(xclave clave){
    CYBIT comando_ok=pdTRUE;
    uint8 indice = 0;
    
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_CLAVE;indice++){
        if(EEPROM_WriteByte(clave.clave[indice],DIRECCION_CONTRA_TESOR+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* leerContraseñaOperador
********************************************************************************
* Función para obtener la contraseña de Operador
*******************************************************************************/
xclave EEPROM_leerClaveOperador(){
    xclave clave;
    uint8 indice;
    char temp;
    for(indice=0; indice<LONGITUD_CLAVE;indice++){
        temp = EEPROM_ReadByte(DIRECCION_CONTRA_OPER+indice);
        clave.clave[indice]=temp;
    }
    return clave;
}

/*******************************************************************************
* escribirClaveOperador
********************************************************************************
* Función para modificar la contraseña del operador
*******************************************************************************/
CYBIT EEPROM_escribirClaveOperador(xclave clave){
    CYBIT comando_ok=pdTRUE;
    uint8 indice = 0;
    
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_CLAVE;indice++){
        if(EEPROM_WriteByte(clave.clave[indice],DIRECCION_CONTRA_OPER+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* leerBrilloPantalla
********************************************************************************
* Función para obtener la configuracion de brillo de la pantalla
*******************************************************************************/
xbrillo leerBrilloPantalla(){
    uint8 byteBrillo;
    xbrillo brillo;
    byteBrillo = EEPROM_ReadByte(DIRECCION_BRILLO_PANTALLA);
    brillo.activo = (byteBrillo&0x01);
    brillo.nivel = byteBrillo>>1;
    return brillo;
}

/*******************************************************************************
* escribirBrilloPantalla
********************************************************************************
* Función para escribir la configuración del brillo la pantalla
*******************************************************************************/
CYBIT escribirBrilloPantalla(xbrillo brillo){
    uint8 byteBrillo;
    CYBIT comando_ok;
    byteBrillo = (brillo.nivel<<1)|brillo.activo;
    
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(byteBrillo,DIRECCION_BRILLO_PANTALLA)==CYRET_SUCCESS){
        comando_ok = pdTRUE;
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
} 

/*******************************************************************************
* leerBrilloMaquina 
********************************************************************************
* Función para obtener la configuracion de brillo de la maquina
*******************************************************************************/
xbrillo leerBrilloMaquina(){
    uint8 byteBrillo;
    xbrillo brillo;
    byteBrillo = EEPROM_ReadByte(DIRECCION_BRILLO_MAQUINA);
    brillo.activo = (byteBrillo&0x01);
    brillo.nivel = byteBrillo>>1;
    return brillo;
}

/*******************************************************************************
* escribirBrilloMaquina
********************************************************************************
* Función para escribir la configuración del brillo la pantalla
*******************************************************************************/
CYBIT escribirBrilloMaquina(xbrillo brillo){
    uint8 byteBrillo;
    CYBIT comando_ok;
    byteBrillo = (brillo.nivel<<1)|brillo.activo;
    
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(byteBrillo,DIRECCION_BRILLO_MAQUINA)==CYRET_SUCCESS){
        comando_ok = pdTRUE;
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
} 

/*******************************************************************************
* leerEscalaPeso
********************************************************************************
* Función para obtener la escala de peso, se debe actualizar luego de hacer una
* nueva calibración del sensor de peso
*******************************************************************************/
float leerEscalaPeso(){
    float escala;
    uint16 factor;
    
    factor = EEPROM_ReadByte(DIRECCION_ESCALA_PESO+1)<<8;
    factor = factor | EEPROM_ReadByte(DIRECCION_ESCALA_PESO);
    escala = (float)factor;
    escala = (float)1/escala;
    return escala;
}

/*******************************************************************************
* escribirEscalaPeso
********************************************************************************
* Función para escribir el factor de escala de peso a partir de la escala
* que haya sido determinada en el dispensador.
*******************************************************************************/
CYBIT escribirEscalaPeso(float escala){
    uint16 factor;
    uint8 byteEscala[2];
    CYBIT comando_ok = pdFALSE;
    escala = 1/escala;
    factor = escala;
    
    byteEscala[0] = factor & 0xFF;
    byteEscala[1] = factor >> 8;
    
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(byteEscala[0],DIRECCION_ESCALA_PESO)==CYRET_SUCCESS){
        if(EEPROM_WriteByte(byteEscala[1],DIRECCION_ESCALA_PESO+1)==CYRET_SUCCESS){
            comando_ok = pdTRUE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
} 

/*******************************************************************************
* leerEscalaTemperatura
********************************************************************************
* Función para obtener la escala de temperatura, se debe actualizar luego de hacer 
* una nueva calibración del sensor de temperatura
*******************************************************************************/
float leerEscalaTemperatura(){
    float escala;
    float factor;
    factor = EEPROM_ReadByte(DIRECCION_ESCALA_TEMP);
    escala = 1/factor;
    return escala;
}

/*******************************************************************************
* escribirEscalaTemperatura
********************************************************************************
* Función para escribir el factor de escala de temperatura a partir de la escala
* que haya sido determinada en el dispensador.
*******************************************************************************/
CYBIT escribirEscalaTemperatura(float escala){
    uint8 factor;
    CYBIT comando_ok;
    factor = 1/escala;
    
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(factor,DIRECCION_ESCALA_TEMP)==CYRET_SUCCESS){
        comando_ok = pdTRUE;
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
} 

/*******************************************************************************
* leerRefrigeracion
********************************************************************************
* Función para obtener la configuracion del sistema de refrigeración
*******************************************************************************/
xTemperatura leerRefrigeracion(){
    uint16 byteRefrigeracion;
    xTemperatura refrigeracion;
    byteRefrigeracion = EEPROM_ReadByte(DIRECCION_TEMPERATURA)<<8;
    byteRefrigeracion = byteRefrigeracion | EEPROM_ReadByte(DIRECCION_TEMPERATURA+1);
    refrigeracion.activado = (byteRefrigeracion&0x01);
    refrigeracion.descanso = (byteRefrigeracion&0x02);
    refrigeracion.gradosC = byteRefrigeracion>>2;
    return refrigeracion;
}

/*******************************************************************************
* escribirRefrigeracion
********************************************************************************
* Función para escribir la configuración del sistema de refrigeración
*******************************************************************************/
CYBIT escribirRefrigeracion(xTemperatura refrigeracion){
    uint8 byteRefrigeracion[2];
    CYBIT comando_ok = pdFALSE;
    refrigeracion.gradosC = (refrigeracion.gradosC<<2)|refrigeracion.activado|(refrigeracion.descanso<<1);
    
    byteRefrigeracion[1] = refrigeracion.gradosC & 0xFF;
    byteRefrigeracion[0] = refrigeracion.gradosC >> 8;
    
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(byteRefrigeracion[0],DIRECCION_TEMPERATURA)==CYRET_SUCCESS){
        if(EEPROM_WriteByte(byteRefrigeracion[1],DIRECCION_TEMPERATURA+1)==CYRET_SUCCESS){
            comando_ok = pdTRUE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* leerHoras
********************************************************************************
* Función para leer las horas de máquina
*******************************************************************************/
xHorasMaquina leerHoras(){
    uint8 i=0;
    uint8 byteHora[LONGITUD_HORAS];
    xHorasMaquina horas;
    
    for(i=0;i<LONGITUD_HORAS;i++){
        byteHora[i]=EEPROM_ReadByte(DIRECCION_HORAS+i);
    }
    horas.horas = byteHora[2]<<16;
    horas.horas = horas.horas | (byteHora[1]<<8);
    horas.horas = horas.horas | byteHora[0];
    
    horas.mediasHoras=horas.horas & 0x03;
    horas.horas = horas.horas >> 2;
    
    return horas;
}

/*******************************************************************************
* escribirHoras
********************************************************************************
* Función para escribir las horas de máquina
*******************************************************************************/
CYBIT escribirHoras(uint8 mediasHoras){
    int horas;
    uint8 indice = 0;
    uint8 byteHora;
    CYBIT comando_ok;
    xHorasMaquina horasGuardadas;
    horasGuardadas = leerHoras();
    horasGuardadas.mediasHoras += mediasHoras;
    while(horasGuardadas.mediasHoras>=2){
        horasGuardadas.horas++;
        horasGuardadas.mediasHoras -= 2;
    }
    horas = horasGuardadas.horas << 2;
    horas = horas | horasGuardadas.mediasHoras;
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_HORAS;indice++){
        byteHora = horas & 0xFF;
        horas = horas >> 8;
        if(EEPROM_WriteByte(byteHora,DIRECCION_HORAS+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}

CYBIT escribirNumeroHoras(uint32 horas){
    uint8 indice = 0;
    uint8 byteHora;
    CYBIT comando_ok;
    horas = horas << 2;
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_HORAS;indice++){
        byteHora = horas & 0xFF;
        horas = horas >> 8;
        if(EEPROM_WriteByte(byteHora,DIRECCION_HORAS+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* estado de devolucion automática
********************************************************************************
* Función para leer o escribir el estado de la devolucion de dinero automática
*******************************************************************************/
/*CYBIT leerEstadoDevolucion(){
    uint8 banderas = EEPROM_ReadByte(DIRECCION_BANDERAS);
    CYBIT devolucionAuto = banderas & 0x01;
    return devolucionAuto;
}

CYBIT escribirEstadoDevolucion(CYBIT devolucionAuto){
    uint8 banderas = EEPROM_ReadByte(DIRECCION_BANDERAS) & MASCARA_DEVOLUCION_AUTO;
    uint8 byteEstado = banderas | devolucionAuto;
    CYBIT comando_ok = pdTRUE;
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(byteEstado,DIRECCION_BANDERAS)!=CYRET_SUCCESS){
        comando_ok=pdFALSE;
    }
    return comando_ok;
}*/

/*******************************************************************************
* estado de telemetria
********************************************************************************
* Función para leer o escribir el estado de la telemetria, si se encuentra activa o no
*******************************************************************************/
CYBIT leerEstadoTelemetria(){
    uint8 banderas = EEPROM_ReadByte(DIRECCION_BANDERAS);
    CYBIT activa = (banderas & 0x02)>>1;
    return activa;
}

CYBIT escribirEstadoTelemetria(CYBIT activa){
    uint8 banderas = EEPROM_ReadByte(DIRECCION_BANDERAS) & MASCARA_ESTADO_TELEMETRIA;
    uint8 byteEstado = banderas | (activa<<1);
    CYBIT comando_ok = pdTRUE;
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(byteEstado,DIRECCION_BANDERAS)!=CYRET_SUCCESS){
        comando_ok=pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* estado de telemetria
********************************************************************************
* Función para leer o escribir el estado de la telemetria, si se encuentra activa o no
*******************************************************************************/
CYBIT leerExistenciaTelemetria(){
    uint8 banderas = EEPROM_ReadByte(DIRECCION_BANDERAS);
    CYBIT existencia = (banderas & 0x04)>>2;
    return existencia;
}

CYBIT escribirExistenciaTelemetria(CYBIT existencia){
    uint8 banderas = EEPROM_ReadByte(DIRECCION_BANDERAS) & MASCARA_EXISTENCIA_TELEMETRIA;
    uint8 byteEstado = banderas | (existencia<<2);
    CYBIT comando_ok = pdTRUE;
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(byteEstado,DIRECCION_BANDERAS)!=CYRET_SUCCESS){
        comando_ok=pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* tiempos de funcionamiento de las bandejas
********************************************************************************
* Función para leer o escribir los tiempos de activacion de las bandejas
*******************************************************************************/
xtiempoComplementario EEPROM_leerTiempos(uint16 tiempos[]){
    uint8 indice = 0;
    uint8 direccion = 0;
    xtiempoComplementario paramMotores;
    for(indice=1;indice<7;indice++){
        direccion = ((indice-1)*2);
        tiempos[indice-1]= EEPROM_ReadByte(DIRECCION_TIEMPOS+direccion) << 8;
        tiempos[indice-1]= tiempos[indice-1] | EEPROM_ReadByte(DIRECCION_TIEMPOS+direccion+1);
    }
    indice = 7;
    direccion = ((indice-1)*2);
    paramMotores.tiempoMovimiento= EEPROM_ReadByte(DIRECCION_TIEMPOS+direccion) << 8;
    paramMotores.tiempoMovimiento= paramMotores.tiempoMovimiento | EEPROM_ReadByte(DIRECCION_TIEMPOS+direccion+1);
    paramMotores.cicloUtil= EEPROM_ReadByte(DIRECCION_TIEMPOS+direccion+2) << 8;
    paramMotores.cicloUtil= paramMotores.cicloUtil | EEPROM_ReadByte(DIRECCION_TIEMPOS+direccion+3);
    return paramMotores;
}

CYBIT EEPROM_escribirTiempos(uint16 tiempos[], uint16 tiempoMovimiento, uint16 cicloUtil){
    uint8 indice = 0;
    uint8 byteTemp = 0;
    uint8 direccion = 0;
    CYBIT comando_ok = pdFALSE;
    EEPROM_UpdateTemperature();
    for(indice=1;indice<7;indice++){
        direccion = ((indice-1)*2);
        byteTemp = tiempos[indice-1]>>8;
        if(EEPROM_WriteByte(byteTemp,DIRECCION_TIEMPOS+direccion)==CYRET_SUCCESS){
            byteTemp = tiempos[indice-1] & 0xFF;
            if(EEPROM_WriteByte(byteTemp,DIRECCION_TIEMPOS+direccion+1)==CYRET_SUCCESS){
                comando_ok=pdTRUE;
            }
        }
    }
    if(comando_ok == pdTRUE){
        comando_ok = pdFALSE;
        indice = 7;
        direccion = ((indice-1)*2);
        byteTemp = tiempoMovimiento>>8;
        if(EEPROM_WriteByte(byteTemp,DIRECCION_TIEMPOS+direccion)==CYRET_SUCCESS){
            byteTemp = tiempoMovimiento & 0xFF;
            if(EEPROM_WriteByte(byteTemp,DIRECCION_TIEMPOS+direccion+1)==CYRET_SUCCESS){
                comando_ok=pdTRUE;
            }
        }
        if(comando_ok == pdTRUE){
            comando_ok = pdFALSE;
            indice = 8;
            direccion = ((indice-1)*2);
            byteTemp = cicloUtil>>8;
            if(EEPROM_WriteByte(byteTemp,DIRECCION_TIEMPOS+direccion)==CYRET_SUCCESS){
                byteTemp = cicloUtil & 0xFF;
                if(EEPROM_WriteByte(byteTemp,DIRECCION_TIEMPOS+direccion+1)==CYRET_SUCCESS){
                    comando_ok=pdTRUE;
                }
            }
        }
    }
    return comando_ok;
}

/*******************************************************************************
* configuracion de las bandejas
********************************************************************************
* Función para leer o escribir las configuraciones de habilitacion y cantidad 
* de motores por bandeja
*******************************************************************************/
xBandejas EEPROM_leerBandejas(){
    uint8 indice = 0;
    uint8 byteLectura = 0;
    xBandejas configBandejas;
    configBandejas.numBandejas = 0;
    for(indice=0;indice<LONGITUD_BANDEJAS;indice++){
        byteLectura = EEPROM_ReadByte(DIRECCION_BANDEJAS+indice);
        configBandejas.numMotores[indice] = byteLectura & 0x7F;
        configBandejas.estado[indice] = byteLectura >> 7;
        if(configBandejas.estado[indice]==1){
            configBandejas.numBandejas++;
        }
        byteLectura = 0;
    }
    return configBandejas;
}

CYBIT EEPROM_escribirBandejas(xBandejas configBandejas){
    uint8 indice = 0;
    uint8 byte = 0;
    CYBIT comando_ok = pdTRUE;
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_BANDEJAS;indice++){ 
        byte = configBandejas.estado[indice]<<7;
        byte = byte | configBandejas.numMotores[indice];
        if(EEPROM_WriteByte(byte,DIRECCION_BANDEJAS+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        } 
        byte = 0;
    }
    return comando_ok;
}

/*******************************************************************************
* configuracion de billetes aceptados
********************************************************************************
* Función para leer o escribir las configuraciones de habilitacion de billetes
* en particular que deban ser aceptados
*******************************************************************************/
uint16 EEPROM_leerBilletesAceptados(){
    uint16 billetes = EEPROM_ReadByte(DIRECCION_BILLETES_ACEPTADOS+1)<<8;
    billetes = EEPROM_ReadByte(DIRECCION_BILLETES_ACEPTADOS)|billetes;
    return billetes;
}

CYBIT EEPROM_escribirBilletesAceptados(uint16 billetes){
    uint8 indice = 0;
    uint8 byteBillete;
    CYBIT comando_ok = pdTRUE;
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_BILLETES_ACEPTADOS;indice++){
        byteBillete = billetes & 0xFF;
        billetes = billetes >> 8;
        if(EEPROM_WriteByte(byteBillete,DIRECCION_BILLETES_ACEPTADOS+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* configuracion de billetes retenidos
********************************************************************************
* Función para leer o escribir las configuraciones de retencion de billetes
* en el stacker
*******************************************************************************/
uint16 EEPROM_leerBilletesRetenidos(){
    uint16 billetes = EEPROM_ReadByte(DIRECCION_BILLETES_RETENIDOS+1)<<8;
    billetes = EEPROM_ReadByte(DIRECCION_BILLETES_RETENIDOS)|billetes;
    return billetes;
}

CYBIT EEPROM_escribirBilletesRetenidos(uint16 billetes){
    uint8 indice = 0;
    uint8 byteBillete;
    CYBIT comando_ok = pdTRUE;
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_BILLETES_RETENIDOS;indice++){
        byteBillete = billetes & 0xFF;
        billetes = billetes >> 8;
        if(EEPROM_WriteByte(byteBillete,DIRECCION_BILLETES_RETENIDOS+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* configuracion de billetes reciclados
********************************************************************************
* Función para leer o escribir las configuraciones de devolucion de billetes
*******************************************************************************/
uint16 EEPROM_leerBilletesReciclados(){
    uint16 billetes = EEPROM_ReadByte(DIRECCION_BILLETES_RECICLADOS+1)<<8;
    billetes = EEPROM_ReadByte(DIRECCION_BILLETES_RECICLADOS)|billetes;
    return billetes;
}

CYBIT EEPROM_escribirBilletesReciclados(uint16 billetes){
    uint8 indice = 0;
    uint8 byteBillete;
    CYBIT comando_ok = pdTRUE;
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_BILLETES_RECICLADOS;indice++){
        byteBillete = billetes & 0xFF;
        billetes = billetes >> 8;
        if(EEPROM_WriteByte(byteBillete,DIRECCION_BILLETES_RECICLADOS+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* habilitacion de funciones de reciclador
********************************************************************************
* Función para leer o escribir la habiltiación de funciones de reciclador
*******************************************************************************/
CYBIT EEPROM_leerHabilitacionReciclador(){
    uint8 banderas = EEPROM_ReadByte(DIRECCION_BANDERAS);
    CYBIT habilitacion = (banderas & 0x80)>>7;
    return habilitacion;
}

CYBIT EEPROM_escribirHabilitacionReciclador(CYBIT habilitado){
    uint8 banderas = EEPROM_ReadByte(DIRECCION_BANDERAS) & MASCARA_RECICLADOR;
    uint8 byteEstado = banderas | (habilitado<<7);
    CYBIT comando_ok = pdTRUE;
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(byteEstado,DIRECCION_BANDERAS)!=CYRET_SUCCESS){
        comando_ok=pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* leerHisteresisSuperior
********************************************************************************
* Función para Leer los grados por encima de la temperatura deseada para encender el compresor
*******************************************************************************/
uint16 EEPROM_leerHisterSuper(){
    uint16 lecturaGrados = 0;
    
    lecturaGrados = EEPROM_ReadByte(DIRECCION_HISTER_SUPER)<<8;
    lecturaGrados = lecturaGrados | EEPROM_ReadByte(DIRECCION_HISTER_SUPER+1);
    if(lecturaGrados==0){
        lecturaGrados = RANGO_SUPERIOR;
    }
    return lecturaGrados;
}

/*******************************************************************************
* escribirHisteresisSuperior
********************************************************************************
* Función para escribir los grados por encima de la temperatura deseada para encender el compresor
*******************************************************************************/
CYBIT EEPROM_escribirHisterSuper(uint16 gradosHisterSuper){
    uint8 bytesGrados[2];
    CYBIT comando_ok = pdFALSE;

    bytesGrados[0] = gradosHisterSuper & 0xFF;
    bytesGrados[1] = gradosHisterSuper >> 8;
    
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(bytesGrados[1],DIRECCION_HISTER_SUPER)==CYRET_SUCCESS){
        if(EEPROM_WriteByte(bytesGrados[0],DIRECCION_HISTER_SUPER+1)==CYRET_SUCCESS){
            comando_ok = pdTRUE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* leerHisteresisInferior
********************************************************************************
* Función para Leer los grados por debajo de la temperatura deseada para encender el compresor
*******************************************************************************/
uint16 EEPROM_leerHisterInfer(){
    uint16 lecturaGrados = 0;
    
    lecturaGrados = EEPROM_ReadByte(DIRECCION_HISTER_INFER)<<8;
    lecturaGrados = lecturaGrados | EEPROM_ReadByte(DIRECCION_HISTER_INFER+1);
    if(lecturaGrados==0){
        lecturaGrados = RANGO_INFERIOR;
    }
    return lecturaGrados;
}

/*******************************************************************************
* escribirHisteresisInferior
********************************************************************************
* Función para escribir los grados por debajo de la temperatura deseada para encender el compresor
*******************************************************************************/
CYBIT EEPROM_escribirHisterInfer(uint16 gradosHisterInfer){
    uint8 bytesGrados[2];
    CYBIT comando_ok = pdFALSE;

    bytesGrados[0] = gradosHisterInfer & 0xFF;
    bytesGrados[1] = gradosHisterInfer >> 8;
    
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(bytesGrados[1],DIRECCION_HISTER_INFER)==CYRET_SUCCESS){
        if(EEPROM_WriteByte(bytesGrados[0],DIRECCION_HISTER_INFER+1)==CYRET_SUCCESS){
            comando_ok = pdTRUE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* leerHorasDescanso
********************************************************************************
* Función para Leer las horas de descanso del compresor
*******************************************************************************/
uint8 EEPROM_leerHorasDescanso(){
    uint8 horas;
    horas = EEPROM_ReadByte(DIRECCION_HORAS_DESCANSO);
    if(horas==0){
        horas = HORAS_DESCANSO_COMPRESOR;    
    }
    return horas;
}

/*******************************************************************************
* escribirHorasDescanso
********************************************************************************
* Función para escribir las horas de descanso del compresor
*******************************************************************************/
CYBIT EEPROM_escribirHorasDescanso(uint8 HorasDescanso){
    CYBIT comando_ok = pdFALSE;

    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(HorasDescanso,DIRECCION_HORAS_DESCANSO)==CYRET_SUCCESS){
          comando_ok = pdTRUE;
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* leerMinutosDescanso
********************************************************************************
* Función para Leer los minutos de descanso del compresor
*******************************************************************************/
uint8 EEPROM_leerMinutosDescanso(){
    uint8 minutos;
    minutos = EEPROM_ReadByte(DIRECCION_MINUTOS_DESCANSO);
    if(minutos==0){
        minutos = TIEMPO_DESCANSO_REFRIGERACION;
    }
    return minutos;
}

/*******************************************************************************
* escribirMinutosDescanso
********************************************************************************
* Función para escribir los minutos de descanso del compresor
*******************************************************************************/
CYBIT EEPROM_escribirMinutosDescanso(uint8 MinutosDescanso){
    CYBIT comando_ok = pdFALSE;
    
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(MinutosDescanso,DIRECCION_MINUTOS_DESCANSO)==CYRET_SUCCESS){
          comando_ok = pdTRUE;
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* leerPesoMinimo
********************************************************************************
* Función para Leer el peso minimo detectado por el sensor de peso
*******************************************************************************/
uint16 EEPROM_leerPesoMinimo(){
    uint16 lecturaPesoMini = 0;
    
    lecturaPesoMini = EEPROM_ReadByte(DIRECCION_PESO_MINIMO)<<8;
    lecturaPesoMini = lecturaPesoMini | EEPROM_ReadByte(DIRECCION_PESO_MINIMO+1);
    if(lecturaPesoMini==0){
        lecturaPesoMini = PESO_MINIMO;
    }
    return lecturaPesoMini;
}

/*******************************************************************************
* escribirPesoMinimo
********************************************************************************
* Función para escribir el peso minimo detectado por el sensor de peso
*******************************************************************************/
CYBIT EEPROM_escribirPesoMinimo(uint16 PesoMinimo){
    uint8 bytesPesoMini[2];
    CYBIT comando_ok = pdFALSE;

    bytesPesoMini[0] = PesoMinimo & 0xFF;
    bytesPesoMini[1] = PesoMinimo >> 8;
    
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(bytesPesoMini[1],DIRECCION_PESO_MINIMO)==CYRET_SUCCESS){
        if(EEPROM_WriteByte(bytesPesoMini[0],DIRECCION_PESO_MINIMO+1)==CYRET_SUCCESS){
            comando_ok = pdTRUE;
        }
    }
    else{
        comando_ok = pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* leerClaveUsuario
********************************************************************************
* Función para obtener la contraseña de un usuario
*******************************************************************************/
xDatosUsuario EEPROM_leerCedulaUsuario(uint8 identificador){
    xDatosUsuario datos;
    int offset = (identificador-1)*LONGITUD_PARAMETROS_USUARIO;
    uint8 indice;
    char temp;
    for(indice=0; indice<LONGITUD_CLAVE_USUARIO;indice++){
        temp = EEPROM_ReadByte(DIRECCION_BASE_USUARIOS+offset+indice);
        datos.clave[indice]=temp;
    }
    return datos;
}

/*******************************************************************************
* escribirClaveUsuario
********************************************************************************
* Función para modificar la contraseña del usuario
*******************************************************************************/
CYBIT EEPROM_escribirCedulaUsuario(char* clave, uint8 identificador){
    CYBIT comando_ok=pdTRUE;
    uint8 indice = 0;
    int offset = (identificador-1)*LONGITUD_PARAMETROS_USUARIO;
    
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_CLAVE_USUARIO;indice++){
        if(EEPROM_WriteByte(clave[indice],DIRECCION_BASE_USUARIOS+offset+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}  

/*******************************************************************************
* leerNombreUsuario
********************************************************************************
* Función para obtener la contraseña de un usuario
*******************************************************************************/
xDatosUsuario EEPROM_leerNombreUsuario(uint8 identificador){
    xDatosUsuario datos;
    uint8 indice;
    char temp;
    int offset = (identificador-1)*LONGITUD_PARAMETROS_USUARIO;
    
    for(indice=0; indice<LONGITUD_NOMBRE_USUARIO;indice++){
        temp = EEPROM_ReadByte(DIRECCION_BASE_USUARIOS+offset+LONGITUD_CLAVE_USUARIO+indice);
        datos.nombre[indice]=temp;
    }
    return datos;
}

/*******************************************************************************
* escribirNombreUsuario
********************************************************************************
* Función para modificar el nombre del usuario
*******************************************************************************/
CYBIT EEPROM_escribirNombreUsuario(char* nombre, uint8 identificador){
    CYBIT comando_ok=pdTRUE;
    uint8 indice = 0;
    int offset = (identificador-1)*LONGITUD_PARAMETROS_USUARIO;
    
    EEPROM_UpdateTemperature();
    for(indice=0;indice<LONGITUD_NOMBRE_USUARIO;indice++){
        if(EEPROM_WriteByte(nombre[indice],DIRECCION_BASE_USUARIOS+offset+LONGITUD_CLAVE_USUARIO+indice)!=CYRET_SUCCESS){
            comando_ok=pdFALSE;
        }
    }
    return comando_ok;
}

/*******************************************************************************
* EstadoBloqueo
********************************************************************************
* Funciónes para verificar si una maquina puede ser bloqueada o no
*******************************************************************************/
CYBIT EEPROM_leerBloqueo(){
    uint8 banderas = EEPROM_ReadByte(DIRECCION_BANDERAS+1);
    CYBIT bloqueo = (banderas & 0x01);
    return bloqueo;
}

CYBIT EEPROM_escribirBloqueo(CYBIT habilitado){
    uint8 banderas = EEPROM_ReadByte(DIRECCION_BANDERAS+1) & MASCARA_BLOQUEO;
    uint8 byteEstado = banderas | habilitado;
    CYBIT comando_ok = pdTRUE;
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(byteEstado,DIRECCION_BANDERAS+1)!=CYRET_SUCCESS){
        comando_ok=pdFALSE;
    }
    return comando_ok;
}

/*******************************************************************************
* rutinas de lectura y escritura de usuarios
********************************************************************************
* Función para leer o escribir los parametros de usuarios
*******************************************************************************/
/*xUsuario EEPROM_leerUsuario(uint8 usuario){
    uint8 indice = 0;
    uint8 direccion = 0;
    xUsuario usuarioLeido;
    usuarioLeido.
    for(indice=1;indice<8;indice++){
        direccion = ((indice-1)*2);
        tiempos[indice-1]= EEPROM_ReadByte(DIRECCION_TIEMPOS+direccion) << 8;
        tiempos[indice-1]= tiempos[indice-1] | EEPROM_ReadByte(DIRECCION_TIEMPOS+direccion+1);
    }
    indice = 9;
    direccion = ((indice-1)*2);
    tiempoMovimiento= EEPROM_ReadByte(DIRECCION_TIEMPOS+direccion) << 8;
    tiempoMovimiento= tiempoMovimiento | EEPROM_ReadByte(DIRECCION_TIEMPOS+direccion+1);
    return tiempoMovimiento;
}

CYBIT EEPROM_escribirTiempos(uint16 tiempos[], uint16 tiempoMovimiento){
    uint8 indice = 0;
    uint8 byteTemp = 0;
    uint8 direccion = 0;
    CYBIT comando_ok = pdFALSE;
    EEPROM_UpdateTemperature();
    for(indice=1;indice<8;indice++){
        direccion = ((indice-1)*2);
        byteTemp = tiempos[indice-1]>>8;
        if(EEPROM_WriteByte(byteTemp,DIRECCION_TIEMPOS+direccion)==CYRET_SUCCESS){
            byteTemp = tiempos[indice-1] & 0xFF;
            if(EEPROM_WriteByte(byteTemp,DIRECCION_TIEMPOS+direccion+1)==CYRET_SUCCESS){
                comando_ok=pdTRUE;
            }
        }
    }
    if(comando_ok == pdTRUE){
        comando_ok = pdFALSE;
        indice = 9;
        direccion = ((indice-1)*2);
        byteTemp = tiempoMovimiento>>8;
        if(EEPROM_WriteByte(byteTemp,DIRECCION_TIEMPOS+direccion)==CYRET_SUCCESS){
            byteTemp = tiempoMovimiento & 0xFF;
            if(EEPROM_WriteByte(byteTemp,DIRECCION_TIEMPOS+direccion+1)==CYRET_SUCCESS){
                comando_ok=pdTRUE;
            }
        }
    }
    return comando_ok;
}*/