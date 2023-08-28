/* ==============================================================
 *
 * Copyright EKIA Technology S.A.S, 2018
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY.
 *
 * Bootloader 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 * ==============================================================
;FUNCIONES PROPIAS DE LA MEMORIA SD
===============================================================*/

#include <string.h>
#include "sdBoot.h"
#include "FS.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cypins.h"
#include "Bootloader.h"
#include "LED_ext.h"
#include "Clock_lento.h"
#include "LED_DEBUG.h"

#define CYBTLDR_FROW_SIZE           CYDEV_FLS_ROW_SIZE
uint8 flashBufferX[CYBTLDR_FROW_SIZE];

FS_FILE *archivoPrograma;

CYBIT habilitarSD(){
    CYBIT comando_ok=0;
    FS_Init();
    FS_FAT_SupportLFN();
    FS_ConfigUpdateDirOnWrite(1);
    Clock_lento_Enable();
    return comando_ok;
}

unsigned char strhex(char valor){
    if ('0' <= valor && valor <= '9')
        return (unsigned char)(valor - '0');
    if ('a' <= valor && valor <= 'f')
        return (unsigned char)(10 + valor - 'a');
    if ('A' <= valor && valor <= 'F')
        return (unsigned char)(10 + valor - 'A');
    return 0;
}

int interpretarCabecera(unsigned int bufSize, unsigned char* buffer, unsigned long* siliconId, unsigned char* siliconRev){
    int err = CYRET_ERR_LENGTH;

    if (bufSize == LARGO_LINEA) //8-silicon id, 2-silicon rev
    {
        *siliconId = (strhex(buffer[0]) << 28) | (strhex(buffer[1]) << 24) | 
            (strhex(buffer[2]) << 20) | (strhex(buffer[3]) << 16) | 
            (strhex(buffer[4]) << 12) | (strhex(buffer[5]) << 8) | 
            (strhex(buffer[6]) << 4) | strhex(buffer[7]);
        *siliconRev = (strhex(buffer[8]) << 4) | strhex(buffer[9]);
        err = CYRET_SUCCESS;
    }
    return err;
}

int leerLinea(char* buffer){
    int err = CYRET_ERR_FILE;

    if (NULL != archivoPrograma && FS_FEof(archivoPrograma)!=1){//Fin de archivo no alcanzado
        if (FS_Read(archivoPrograma,buffer,LARGO_LINEA)==LARGO_LINEA){ //Leer la linea entera. 
			//Ubique el puntero dos bytes por delante para el salto de linea despues del checksum (ultimo Byte)
            err=FS_FSeek(archivoPrograma,2, FS_SEEK_CUR);
        } 
        else {
            err = CYRET_ERR_EOF;
		}
    }

    return err;
}

int interpretarLinea(uint16 bufSize, char* buffer, unsigned char* arrayId, uint16* rowNum, unsigned char* rowData, uint16* size, unsigned char* checksum){
    unsigned int i, j;
    uint16 size2;
    int err = CYRET_ERR_CMD;

    if (bufSize <= LARGO_MIN){
        err = CYRET_ERR_LENGTH;
    }
    else if (buffer[0] == ':'){
        *arrayId = (strhex(buffer[1]) << 4) | strhex(buffer[2]);
        *rowNum = (strhex(buffer[3]) << 12) | (strhex(buffer[4]) << 8) |(strhex(buffer[5]) << 4) | strhex(buffer[6]);
        size2 = (strhex(buffer[7]) << 12) | (strhex(buffer[8]) << 8) |(strhex(buffer[9]) << 4) | (strhex(buffer[10]));
        *size = size2;
        *checksum = (strhex(buffer[bufSize - 2]) << 4) | strhex(buffer[bufSize - 1]);

        if ((size2 * 2) == (unsigned short)(bufSize - LARGO_MIN)){
            for (i=0; i<size2; i++){
                j = DATA_OFFSET + (i*2);
                rowData[i] = (strhex(buffer[j]) << 4) | strhex(buffer[j + 1]);
            }
            err = CYRET_SUCCESS;
        }
        else{
            //LED_DEBUG_Write(1);
            err = CYRET_ERR_DATA;
        }
    }
	buffer=NULL;
    return err;
}

int SD_Bootload(char * file){
	unsigned char cyacd_cabecera[LARGO_CABECERA];
	char cyacd_linea[LARGO_LINEA];
	unsigned char cyacd_arregloId;
	uint16 cyacd_direccion;
	unsigned char cyacd_datos[288];
	uint16 cyacd_tamano;
	unsigned char cyacd_checksum;
	unsigned long siliconId;
	unsigned char siliconRev;
    CYBIT estado = 1;
    char ruta[30];
    
	int err=CYRET_ERR_CMD;
	habilitarSD();
	/*Inicialice el mecanismo de escritura en la flash*/
	if(!(CYRET_SUCCESS == CySetTemp() && CYRET_SUCCESS == CySetFlashEEBuffer(flashBufferX))){
	/*Si la escritura en la Flash no pudo inicializarse correctamente
    reinicie el sistema y reintente*/
        CySoftwareReset();
	}
	
	/*Inicializar sistema de archivos*/
	//FS_Init();
	archivoPrograma = FS_FOpen(file, "r");
	
	/*Se encontró archivo valido*/
	if ((NULL !=  archivoPrograma) && (FS_FEof(archivoPrograma)!=1)){
		err=FS_Read(archivoPrograma,cyacd_cabecera,LARGO_CABECERA);
		/* leer la cabecera del archivo*/
        
		if (err!=0){ 
            /* Colocar el puntero por delante de 4, para ubicar el inicio de la primera fila */
		    err=FS_FSeek(archivoPrograma,4, FS_SEEK_CUR);
		
		    /*Interpretar la cabecera para obtener el SiliconID y SiliconRev*/
		    err=interpretarCabecera(10,cyacd_cabecera,&siliconId,&siliconRev);
		
    		/*Heres where you should check the SiliconID and Rev.*/
    		/*
    		if(CYSWAP_ENDIAN32(CYDEV_CHIP_JTAG_ID)==siliconID && CYDEV_CHIP_REV_EXPECT==siliconRev){
    		}else{
    		}*/
    	}else{
    		/*Se tiene un archivo nulo o se ha alcanzado el final del archivo*/
           	return CYRET_ERR_EOF;
    	}
		
		while(1){
		    LED_ext_Write(estado);
            estado = ~estado;
		    /*Lea la primera fila luego de la cabecera.*/
    		if(leerLinea(cyacd_linea)!=CYRET_SUCCESS){
    			break;
    		}
		    /*interprete la linea para obtener direccion, datos, tamaño y el cheksum*/
    		if(interpretarLinea(LARGO_LINEA,cyacd_linea,&cyacd_arregloId,&cyacd_direccion,&cyacd_datos[0],&cyacd_tamano,&cyacd_checksum)!=CYRET_SUCCESS){
                break;
    		}
    		/*Escriba la linea en la flash*/
    		CyWriteRowFull(cyacd_arregloId,cyacd_direccion,cyacd_datos,cyacd_tamano);
		}
        
        //sprintf(ruta,"%s\\%s",pName_Dir_Conf,file);
        FS_Remove("\\CONFIGURACION\\principal.cyacd"); //elimine el archivo, si ya existe una copia
		if(FS_Move("principal.cyacd","\\CONFIGURACION\\principal.cyacd")!=CYRET_SUCCESS){ //muevalo a la nueva ubicación
		/*Reinicie sistema*/
		    CySoftwareReset();
        }
        
		err=FS_FClose(archivoPrograma);
	}
    return err;
}

/* [] END OF FILE */
