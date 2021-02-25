/**
 ****************************************************************************************
 *
 * @file   osapp_calendar.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2019-01-11 15:28
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_UTILS_OSAPP_CALENDAR_H_
#define APP_FREERTOS_UTILS_OSAPP_CALENDAR_H_


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
typedef struct
{
    int   ms;
    int   sec;
    int   min;
    int   hour;
    int   mday;
    int   mon;
    int   year;
}tm_date;

int set_system_time(tm_date stm);
int get_system_time(tm_date *stm);
uint64_t time_set_date_to_tai_seconds(tm_date *stm);


#endif /* APP_FREERTOS_UTILS_OSAPP_CALENDAR_H_ */ 

