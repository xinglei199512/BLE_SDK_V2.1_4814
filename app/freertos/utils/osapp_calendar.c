/**
 ****************************************************************************************
 *
 * @file   osapp_calendar.c
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

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"
#include "osapp_calendar.h"
/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

static volatile uint64_t time_tai_seconds = 0;
static volatile uint8_t time_ms = 0;
static volatile uint32_t tick_count_current;
static volatile uint32_t tick_count_last;
static TimerHandle_t time_Timer;


void set_tai_seconds_to_date(tm_date *stm)
{
    int64_t L, B;
    int32_t D, H, C, X, Q, Z, V;
    int8_t A, J, K;
    uint32_t tick_count = xTaskGetTickCount();

    LOG(3,"set_tai_seconds_to_date:%x %x %x\n", tick_count, tick_count_last, tick_count_current);

    if(tick_count >= tick_count_last){
        L = time_tai_seconds + (uint64_t)((tick_count - tick_count_current) * (1.0 / configTICK_RATE_HZ));
        stm->ms = ((uint16_t)((tick_count - tick_count_current) * (1000 / configTICK_RATE_HZ)) + time_ms) % 1000;
    }else {
        L = time_tai_seconds + (uint64_t)((0xffffffff + tick_count - tick_count_current) * (1.0 / configTICK_RATE_HZ));
        stm->ms = ((uint16_t)((0xffffffff + tick_count - tick_count_current) * (1000 / configTICK_RATE_HZ)) + time_ms) % 1000;
        tick_count_last = tick_count_current = tick_count;
        time_tai_seconds = L;
    }

    D = (int)(L / 86400);
    stm->hour = (int)(((L - D * 86400) / 3600) % 24);
    stm->min = (int)((L - D * 86400 - stm->hour * 3600) / 60 % 60);
    stm->sec = (int)((L - D * 86400 - stm->hour * 3600 - stm->min * 60) % 60);

    B = D + 730119;
    Q = B % 146097;
    C = (int)(Q / 36524);
    H = Q % 36524;
    X = (int)((H % 1461) / 365);
    stm->year = (int)((B / 146097) * 400 
                                + C * 100
                                + (int)(H / 1461) * 4 
                                + X
                                + (!((C == 4) || (X == 4)) ? 1 : 0));

    Z = stm->year - 1;
    V = B - 365 * Z - (int)(Z / 4) + (int)(Z / 100) - (int)(Z / 400);

    if((stm->year % 4 == 0 && stm->year % 100 != 0) || stm->year % 400 == 0)
        A = 1;
    else 
        A = 2;

    if(V + A < 61)
        J = 0;
    else
        J = A;

    stm->mon = (int)(((V + J) * 12 + 373) / 367);

    if(stm->mon <= 2)
        K = 0;
    else 
        K = A;

    stm->mday = V + K + 1 - (int)((367 * stm->mon - 362) / 12);

    LOG(3, "set_tai_seconds_to_date tai_seconds:%x %x\n", time_tai_seconds, (time_tai_seconds >> 32) & 0xffffffff);
    LOG(3, "now_time ms:%d\n", stm->ms);
    LOG(3, "second:%d\n", stm->sec);
    LOG(3, "minute:%d\n", stm->min);
    LOG(3, "hour:%d\n", stm->hour);
    LOG(3, "mday:%d\n", stm->mday);
    LOG(3, "month:%d\n", stm->mon);
    LOG(3, "year:%d\n", stm->year);
}

static void handler_time_calculate_timer(TimerHandle_t thandle)
{
    uint32_t tick_count = xTaskGetTickCount();
    if(tick_count < tick_count_last) {
        time_tai_seconds += (uint64_t)(((uint64_t)(0xffffffff - tick_count_current + tick_count)) * (1.0 / configTICK_RATE_HZ));
        tick_count_current = tick_count;
    }
    LOG(3,"handler_time_calculate_timer:%x %x %x\n", tick_count, tick_count_last, tick_count_current);
    tick_count_last = tick_count;

    xTimerChangePeriod(time_Timer, pdMS_TO_TICKS(0xEA60 * 60), portMAX_DELAY);
}

uint64_t time_set_date_to_tai_seconds(tm_date *stm)
{
    uint64_t days = 0;
    uint8_t month_day[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t leap_year = ((stm->year % 4 == 0) && (stm->year % 100 != 0)) || (stm->year % 400 == 0);
    for(int year = 2000; year < stm->year; year++) {
        if(((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) {
            days += 366;
        }else
            days += 365;
    }
    if(leap_year)
        month_day[2] = 29;
    for(int i = 1; i < stm->mon; i++) {
        days += month_day[i];
    }
    days += stm->mday - 1;
    LOG(3, "time_set_date_to_tai_seconds year:%d hour:%d min:%d\n", stm->year, stm->hour, stm->min);

    return ((days * 24 + stm->hour) * 60 + stm->min) * 60 + stm->sec;
}

int set_system_time(tm_date stm)
{
    tick_count_current = tick_count_last = xTaskGetTickCount();
    LOG(3,"set_system_time:%x %x\n", tick_count_last, tick_count_current);

    time_ms = stm.ms;
    time_tai_seconds = time_set_date_to_tai_seconds(&stm);

    LOG(3, "time_set_date_to_tai_seconds: %d %x %x\n", time_ms, time_tai_seconds, ((time_tai_seconds >> 32) & 0xffffffff));

    if(time_Timer == NULL) {
        time_Timer = xTimerCreate(
                "time", pdMS_TO_TICKS(0xEA60 * 60), pdFALSE, NULL, handler_time_calculate_timer);

        if(time_Timer != NULL)
            xTimerStart(time_Timer, 0);
    }else {
        xTimerChangePeriod(time_Timer, pdMS_TO_TICKS(0xEA60 * 60), portMAX_DELAY);
    }

    return 0;
}

int get_system_time(tm_date *stm)
{
    set_tai_seconds_to_date(stm);

    return 0;
}

