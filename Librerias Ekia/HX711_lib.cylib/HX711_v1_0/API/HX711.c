/* ============================================================================
 * File Name: HX711_v0_0.c
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


#include "`$INSTANCE_NAME`.h" // must specify API prefix in Symbol->Properties->Doc.APIPrefix
#include <`$INSTANCE_NAME`_Pin_SCLK.h>
#include <`$INSTANCE_NAME`_Pin_SDAT.h>
#include "math.h"


#define CHECK_BIT(var, pos) ((var>>pos) & 1)


//====================================
//        initialization
//====================================

static int32 aOffset  = `=$ADC_offset`;                                      // ADC offset
static uint8 aSetGain = `=($ADC_gain==128)? 1 : ($ADC_gain==64)? 3 : 2`;    // 1- gain 128, 3- gain 64, 2-gain 32
static float aFactor  = 1/`=$ADC_factor`;

//====================================
//        private variables
//====================================
static volatile int32 aCount = 0;                      // last retrieved data




//==============================================================================
// Initialize HX711 
//==============================================================================
void `$INSTANCE_NAME`_Start()
{ 
    
    // must read to set gain and to discard first data count
    // set pin SCLK to 0 to reset and enter working mode
    // this also enabled power-up to the bridge
    `$INSTANCE_NAME`_Pin_SCLK_Write(0);   //ensure serial clock low ready for HX711 data retrieval
} 



//==============================================================================
// Read HX711 24-bit DelSig_ADC
// http://www.fadstoobsessions.com/Learning-Electronics/Component-Testing/HX711-ADC-Weigh-Scale.php
//==============================================================================
int32 `$INSTANCE_NAME`_GetResult32()   
{
    //const uint8 delay_us = 1;
    //const uint8 delay_clk = 0;//
    
    int32 result=0;
    uint8 i;
    uint8 pin_read;

    
    for (i=0; i<24; i++) //clock in the data
    { 
        //`$INSTANCE_NAME`_Pin_SCLK_Write(1);  
        `$INSTANCE_NAME`_Pin_SCLK_DR = (1 << `$INSTANCE_NAME`_Pin_SCLK_SHIFT); // write 1
        
        //CyDelayCycles(delay_clk);
        //`$INSTANCE_NAME`_Pin_SCLK_Write(0);  
        `$INSTANCE_NAME`_Pin_SCLK_DR = (0 << `$INSTANCE_NAME`_Pin_SCLK_SHIFT); // write 0

        
        result <<= 1;
        //if (`$INSTANCE_NAME`_Pin_SDAT_Read()) result++;
        //pin_read = `$INSTANCE_NAME`_Pin_SDAT_Read();
        
        CyDelayCycles(0); //48MHz
        pin_read = (`$INSTANCE_NAME`_Pin_SDAT_PS & `$INSTANCE_NAME`_Pin_SDAT_MASK) >> `$INSTANCE_NAME`_Pin_SDAT_SHIFT; // read SDAT
        if (pin_read) result++;

    }

    // //set the gain for next measurement->
    for (i=0; i<aSetGain; i++) { 
        //`$INSTANCE_NAME`_Pin_SCLK_Write(1);  
        ///CyDelayUs(delay_us);
        ///CyDelayCycles(delay_clk);
        //`$INSTANCE_NAME`_Pin_SCLK_Write(0);  
        ///CyDelayUs(1);
        //CyDelayCycles(0);
        
       `$INSTANCE_NAME`_Pin_SCLK_DR = (1 << `$INSTANCE_NAME`_Pin_SCLK_SHIFT); // write 1
        //CyDelayCycles(delay_clk);
        `$INSTANCE_NAME`_Pin_SCLK_DR = (0 << `$INSTANCE_NAME`_Pin_SCLK_SHIFT); // write 0
    }

    //`$INSTANCE_NAME`_Pin_SCLK_Write(0);   //ensure serial clock is low (ready for HX711 data retrieval)
    `$INSTANCE_NAME`_Pin_SCLK_DR = (0 << `$INSTANCE_NAME`_Pin_SCLK_SHIFT); //  (write 0) ensure serial clock low ready for HX711 data retrieval


    if (CHECK_BIT(result,23)) result = result | 0xff000000;
    
    //result -= aOffset;  // subtract ADC offset
    aCount = result;    // store result value for future use
    
    return(result);
}


//==============================================================================
// Check if data is ready for retrieval
// (pin to go low)
//==============================================================================
#if (`=$state_check == sc_poll`)    
uint8 `$INSTANCE_NAME`_GetDataReady()
{
    return !((`$INSTANCE_NAME`_Pin_SDAT_PS & `$INSTANCE_NAME`_Pin_SDAT_MASK) >> `$INSTANCE_NAME`_Pin_SDAT_SHIFT); // !Pin_SDAT_Read()
}
#endif

//==============================================================================
// Getter of last ADC data
//==============================================================================
int32 `$INSTANCE_NAME`_GetCount() { 
    float count = aCount - aOffset;
    count = fabsf(count);
    return (count);    
}


//==============================================================================
// Stop HX711 
//==============================================================================
void `$INSTANCE_NAME`_Stop()
{ 
    //untested
    // set pin SCLK to 1 to send HX711 into hibernation
    // the bridge is also powered down
    // after power down, gain is automatically set to 128
    // if other gain is needed (64), first data must be discarded
    `$INSTANCE_NAME`_Pin_SCLK_Write(1);   //
} 

//==============================================================================
// Convert raw counts to mV scale using gain 
//==============================================================================
float `$INSTANCE_NAME`_Count_to_mV(int32 value)
{
    // A=+/-20mV <--> G=128
    // A=+/-40mV <--> G=64
    // A=+/-80mV <--> G=32
    // result[mV] = 2 * scale[mV] * value / 2^24
    
    const float Vscale = (`$INSTANCE_NAME`_Gain==128)? 20.0 : (`$INSTANCE_NAME`_Gain==64)? 40.0 : 80.0;
    const float coeff = Vscale / 8388608.0;  // 2^23 
    
    return ( (float) value * coeff ); 
} 

//==============================================================================
// Convierte el conteo RAW a escala de gramos
//==============================================================================
float `$INSTANCE_NAME`_Count_to_gr(int32 value)
{
    return ( (float) value * aFactor); 
} 

void `$INSTANCE_NAME`_SetFactor(int32 factor){
    aFactor = 1.0f/(float)factor;
}

int32 `$INSTANCE_NAME`_GetFactor(){
    return (int32)(1.0f/aFactor);
}

//==============================================================================
// Set ADC offset
//==============================================================================
void `$INSTANCE_NAME`_SetOffset(int32 value) { aOffset = value; }

//==============================================================================
// Getter for ADC offset
//==============================================================================
int32 `$INSTANCE_NAME`_GetOffset()  { return aOffset; }

void `$INSTANCE_NAME`_SetZero(){
    aOffset = aCount;
}

/* [] END OF FILE */




