/* ============================================================================
 * File Name: HX711_v0_0.h
 *   Development version of the HX711 component
 *
 * Description:
 *   Implements software interface to the 24-bit DelSig_ADC HX711 board 
 *   Connects single HX711 sensor board.
 *   Multiple sensors can operate at the same time.
 *   Outputs data in raw 32-bit format or absolute voltage.
 *   ADC selectable gain options 32/64/128.
 *   Uses interrupt or polling technique.
 *
 * ============================================================================
 * PROVIDED AS-IS, NO WARRANTY OF ANY KIND, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * FREE TO SHARE, USE AND MODIFY UNDER TERMS: CREATIVE COMMONS - SHARE ALIKE
 * ============================================================================
*/




#ifndef `$INSTANCE_NAME`_H
#define `$INSTANCE_NAME`_H
 
    
#include <project.h>
#include <cytypes.h>
          
#define true  1
#define false 0


/***************************************
*        parametros de solo lectura - read-only parameters
***************************************/  
#define `$INSTANCE_NAME`_Gain       `=$ADC_gain`                // Ganancia del ADC 
#define `$INSTANCE_NAME`_IsrMode    `=$state_check==sc_isr`     // ADC data ready state check (polling: 0, isr: 1)
#define `$INSTANCE_NAME`_Factor     `=$ADC_factor`              // Factor de conversión a gramos  



/***************************************
*        global variables
***************************************/  
    


/***************************************
*        read-only variables
***************************************/  

#if (`=$state_check == sc_poll`)    
#define `$INSTANCE_NAME`_DataReady       `$INSTANCE_NAME`_GetDataReady()       // flag
#endif

#define `$INSTANCE_NAME`_Count           `$INSTANCE_NAME`_GetCount()           // ADC result
#define `$INSTANCE_NAME`_Offset          `$INSTANCE_NAME`_GetOffset()          // ADC offset
    

    
/***************************************
*        Function Prototypes
***************************************/

void  `$INSTANCE_NAME`_Start();                     // start ADC
void  `$INSTANCE_NAME`_Stop();                      // stop ADC
int32 `$INSTANCE_NAME`_GetResult32();               // read ADC data
int32 `$INSTANCE_NAME`_GetCount();                  // ADC result //todo: if negative?  
uint8 `$INSTANCE_NAME`_GetDataReady();              // flag ready read ADC data 
float `$INSTANCE_NAME`_Count_to_mV(int32 value);    // output result to mV
void  `$INSTANCE_NAME`_SetOffset(int32 value);      // set ADC offset
int32 `$INSTANCE_NAME`_GetOffset();                 // get offset
float `$INSTANCE_NAME`_Count_to_gr(int32 value);    // salida de resultado en gramos
void  `$INSTANCE_NAME`_SetFactor(int32 factor);     // definir el factor de escala a gramos
int32 `$INSTANCE_NAME`_GetFactor();                 // obtener el factor de escala a gramos
void  `$INSTANCE_NAME`_SetZero();                   // hacer cero la medida


    
#endif /* `$INSTANCE_NAME`_H */

/* [] END OF FILE */

