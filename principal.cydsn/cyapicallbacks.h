/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#ifndef CYAPICALLBACKS_H
#define CYAPICALLBACKS_H
    
    /*Define your macro callbacks here */
    #include "RTC_RTC.h"
    #include "RTC_EKIA.h"
    #include "general.h"
    
    #define MODULO_SIM800L
    /* Callback function prototypes */
    //void RTC_EverySecondHandler_Callback(void);
    void RTC_RTC_EveryMinuteHandler_Callback(void);
    //void RTC_EveryHourHandler_Callback(void);
    /*void RTC_EveryDayHandler_Callback(void);
    void RTC_EveryMonthHandler_Callback(void);
    void RTC_EveryYearHandler_Callback(void);*/

    /* Callback defines */
    //#define RTC_EVERY_SECOND_HANDLER_CALLBACK
    #define RTC_RTC_EVERY_MINUTE_HANDLER_CALLBACK
    //#define RTC_EVERY_HOUR_HANDLER_CALLBACK
    /*#define RTC_EVERY_DAY_HANDLER_CALLBACK
    #define RTC_EVERY_MONTH_HANDLER_CALLBACK
    #define RTC_EVERY_YEAR_HANDLER_CALLBACK*/
    
#endif /* CYAPICALLBACKS_H */   
/* [] */
