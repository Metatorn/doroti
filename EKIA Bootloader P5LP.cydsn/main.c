/* ===================================================
 *
 * Copyright EKIA Technology SAS, 2019
 * Todos los Derechos Reservados
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * INFORMACION CONFIDENCIAL Y PROPRIETARIA
 * ES PROPIEDAD DE EKIA TECHNOLOGY SAS.
 *
 * Bootloader 1.0
 * Programado por: Diego Felipe Mejia Ruiz
 * Bogotá, Colombia
 *
 * ===================================================*/

#include <project.h>
#include "sdBoot.h"
#include "FS.h"
#include "string.h"
#include "stdio.h"
#include "USB_Bootloader.h"

#define FUENTE                      "principal.cyacd"
#define DIRECCION_PROGRAMACION      (0x7FF) 
#define PROGRAMADO                  100

uint8 EEPROM_leerEstadoProgramacion(){
    uint8 reprogramar = 0;
    return reprogramar= EEPROM_ReadByte(DIRECCION_PROGRAMACION);
}

CYBIT EEPROM_escribirEstadoProgramacion(uint8 estado){
    CYBIT comando_ok = 1;
    EEPROM_UpdateTemperature();
    if(EEPROM_WriteByte(estado,DIRECCION_PROGRAMACION)!=CYRET_SUCCESS){
        comando_ok=0;
    }
    return comando_ok;
}

int main(){
    EEPROM_Start();
    //cargue la imagen de programa si está presente
    if(SDIn_Read()){
        if(SD_Bootload(FUENTE)==CYRET_SUCCESS){
            EEPROM_escribirEstadoProgramacion(PROGRAMADO);
        }
    }
    if(CYRET_SUCCESS != Bootloader_ValidateBootloadable(Bootloader_MD_BTLDB_ACTIVE_0)||
    (EEPROM_leerEstadoProgramacion()!=PROGRAMADO)){
        //habilitarSD();
        //CySoftwareReset();
    }
    //USB_Bootloader_Start(0, USB_Bootloader_DWR_VDDD_OPERATION);
    //if (USB_Bootloader_GetConfiguration()){ //se espera a que sea recononcido el dispositivo USB
    Bootloader_Start();
    //    LED_USB_Write(1);
    //}
    
    //Bootloader_Start();
    /* Bootloader_Start() never returns. */
    for (;;)
    {
    }
}


/* [] END OF FILE */
