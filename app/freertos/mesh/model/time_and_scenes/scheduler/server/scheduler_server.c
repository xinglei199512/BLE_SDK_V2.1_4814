/**
 ****************************************************************************************
 *
 * @file   scheduler_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-21 10:39
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "scheduler_server.h"
#include "time_server.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
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
#define TIMER_MAX_DELAY_SEC (0xffffffff / configTICK_RATE_HZ)
static uint8_t today_second_is_past(scheduler_time_value_t *time_value, tm_date now_time, uint16_t repeat);
static uint8_t today_minute_is_past(scheduler_time_value_t *time_value, tm_date now_time, uint16_t repeat);
static uint8_t today_hour_is_past(scheduler_time_value_t *time_value, tm_date now_time, uint16_t repeat);
static void scheduler_time_handler(scheduler_server_t *server);

static void scheduler_call_user_callback(scheduler_server_t *server, mesh_scheduler_model_evt_type_t type, uint16_t value, uint8_t repeat_flag)
{
    mesh_model_evt_t evt;
    evt.type.scheduler_type = type;
    evt.params.model_scheduler_set.target_value = value;
    evt.params.model_scheduler_set.repeat_flag = repeat_flag;
    if(server->cb)
        server->cb(&evt);
}

static void scheduler_minus_elapsed_time(scheduler_server_t *server, uint64_t elapsed_time, int current_index)
{
    LOG(3, "scheduler_minus_elapsed_time elapsed_time:%x\n", (uint32_t)(elapsed_time & 0xffffffff));
#if 1
    for(int i = 0; i < SCHSDULER_INDEX_MAX; i++) {
        LOG(3, "elapsed_time alarm_time[%d]:%x %x\n", i, (uint32_t)(server->scheduler_time->alarm_time[i] & 0xffffffff), (uint32_t)((server->scheduler_time->alarm_time[i] >> 32) & 0xffffffff));
    }
#endif
    for(int i = 0; i < SCHSDULER_INDEX_MAX; i++) {
        if((server->scheduler_time->alarm_time[i]) && (i != current_index))
            server->scheduler_time->alarm_time[i] -= elapsed_time;
    }
}

static void scheduler_action_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "%s\n", __func__);
}

static void send_scheduler_action_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scheduler_server_t *server = GET_SERVER_MODEL_PTR(scheduler_server_t, model);
    scheduler_action_status_t msg;
    memcpy(&msg, &server->msg_format->action_set, sizeof(scheduler_action_set_t));
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(TIME_AND_SCENE_ONE_OCTET_OPCODE_OFFSET, Scheduler_Action_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(scheduler_action_set_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,scheduler_action_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}

static void scheduler_server_action_timer_handle(mesh_timer_t param)
{
    scheduler_server_t *server = (scheduler_server_t *)mesh_timer_get_associated_data(param);

    LOG(3, "%s alarm_remain_time:%x alarm_time:%x\n", 
            __func__, (uint32_t)(server->scheduler_time->alarm_remain_time & 0xffffffff), (uint32_t)(server->scheduler_time->alarm_time[server->scheduler_time->alarm_num] & 0xffffffff));

    if(server->scheduler_time->alarm_remain_time) {
        uint32_t delay_time;

        if(server->scheduler_time->alarm_remain_time > TIMER_MAX_DELAY_SEC) {
            delay_time = TIMER_MAX_DELAY_SEC;
            server->scheduler_time->alarm_remain_time -= TIMER_MAX_DELAY_SEC;
        }else {
            delay_time = (uint32_t)server->scheduler_time->alarm_remain_time;
            server->scheduler_time->alarm_remain_time = 0;
        }
        mesh_timer_change_period(server->scheduler_time->scheduler_Timer, pdMS_TO_TICKS(delay_time));
    }else {
        uint8_t current_index = 0;
        uint8_t  alarm_count = server->scheduler_time->alarm_count;

        scheduler_minus_elapsed_time(server, server->scheduler_time->alarm_time[server->scheduler_time->alarm_num], -1);

        server->scheduler_time->alarm_num = -1;

        if(server->scheduler_time->scheduler_Timer) {
            mesh_timer_delete(server->scheduler_time->scheduler_Timer);
            server->scheduler_time->scheduler_Timer = NULL;
        }
        server->scheduler_time->last_timeout_flag = 1;
        LOG(3, "1current_alarm_index[%d]:%x alarm_count:%d\n", 
                current_index, server->scheduler_time->current_alarm_index[current_index], alarm_count);
        while(alarm_count--) {
            scheduler_call_user_callback(server, TIME_SCHEDULER_EVT_ACTION_GET, (uint16_t)server->scheduler_time->current_alarm_index[current_index], 0);
            LOG(3, "scheduler_server_action_timer_handle current_alarm_index[%d]:%x\n", 
                    current_index, server->scheduler_time->current_alarm_index[current_index]);

            server->scheduler_time->action_cb(server, server->msg_format->action_set.action);
            server->scheduler_time->current_alarm_index[current_index] = -1;
            server->scheduler_time->alarm_count--;
            if(alarm_count == 0)
                server->scheduler_time->last_timeout_flag = 0;
            scheduler_time_handler(server);
            current_index++;
        }
    }
}

static uint8_t today_second_is_past(scheduler_time_value_t *time_value, tm_date now_time, uint16_t repeat)
{
    if(now_time.sec >= time_value->second) {
        if(repeat & REPEAT_BY_SEC) {
            if(repeat & REPEAT_BY_ANY_SEC) {
                if(now_time.sec == 0x3B)
                    return 1;
                else
                    return 0;
            }else if(repeat & REPEAT_BY_EVERY15_SEC) {
                if(now_time.sec >= 0x2D)
                    return 1;
                else
                    return 0;
            }else {
                if(now_time.sec >= 0x28)
                    return 1;
                else
                    return 0;
            }
        }else
            return 1;
    }
    return 0;
}
static uint8_t today_minute_is_past(scheduler_time_value_t *time_value, tm_date now_time, uint16_t repeat)
{
    if(now_time.min >= time_value->minute) {
        if(repeat & REPEAT_BY_MIN) {
            if(repeat & REPEAT_BY_ANY_MIN) {
                if(now_time.min == 0x3B) {
                    return today_second_is_past(time_value, now_time, repeat);
                }else {
                    return 0;
                }
            }else if(repeat & REPEAT_BY_EVERY15_MIN) {
                if(now_time.min > 0x2D) {
                   return 1; 
                }else if(now_time.min == 0x2D){
                    return today_second_is_past(time_value, now_time, repeat);
                }else {
                    return 0;
                }
            }else {
                if(now_time.min > 0x28) {
                   return 1; 
                }else if(now_time.min == 0x28){
                    return today_second_is_past(time_value, now_time, repeat);
                }else {
                    return 0;
                }
            }
        }
        else {
            if(now_time.min == time_value->minute) {
                return today_second_is_past(time_value, now_time, repeat);
            }else
                return 1;
        }
    }
    return 0;
}
static uint8_t today_hour_is_past(scheduler_time_value_t *time_value, tm_date now_time, uint16_t repeat)
{
    if(now_time.hour >= time_value->hour) {
        if(repeat & REPEAT_BY_ANY_HOUR) {
            if(now_time.hour == 0x17) {
                if(today_minute_is_past(time_value, now_time, repeat))
                    return 1;
                else {
                    return 0;
                }
            }
            return 0;
        }else {
            if(now_time.hour == time_value->hour)
                return today_minute_is_past(time_value, now_time, repeat);
            else
                return 1;
        }
    }
    return 0;
}
static uint8_t today_time_is_past(scheduler_time_value_t *time_value, tm_date now_time, uint16_t repeat)
{
    return today_hour_is_past(time_value, now_time, repeat);
}

static uint32_t get_day_min_tai_second(scheduler_time_value_t *time_value, tm_date now_time, uint16_t repeat_flag, uint8_t *day_tai_flow, uint8_t not_current_day)
{
    uint32_t day_tai_second = 0;
    uint32_t tick_count = xTaskGetTickCount();
    uint32_t hour_tai_second = 0;
    uint32_t min_tai_second = 0;
    uint32_t sec_tai_second = 0;

    do{
        int hour_rand = (tick_count % 0x18);
        if(repeat_flag & REPEAT_BY_ONCE_HOUR) {
            if(hour_rand > now_time.hour) {
                hour_tai_second += hour_rand - now_time.hour;
            }else {
                hour_tai_second += 60 + hour_rand - now_time.hour;
                *day_tai_flow |= TAI_HOUR_FLOW; 
            }
        }else if(repeat_flag & REPEAT_BY_ANY_HOUR){
            if(not_current_day) {
                hour_tai_second += 24 - now_time.hour;
                *day_tai_flow |= TAI_HOUR_FLOW; 
                break;
            }else {
                hour_tai_second += 0;
            }
        }else {
            LOG(3, "time_value->hour%x now_time.hour%x %d\n", (int)time_value->hour, now_time.hour, (int)time_value->hour > now_time.hour);
            if(((int)time_value->hour >= now_time.hour)) {
                hour_tai_second += (int)time_value->hour - now_time.hour;
            }else {
                hour_tai_second += 24 + (int)time_value->hour - now_time.hour;
                *day_tai_flow |= TAI_HOUR_FLOW; 
            }
        }
    }while(0);

    LOG(3, "hour hour_tai_second:%x\n", hour_tai_second);

    do{
        int min_rand = (tick_count % 0x3C);
        LOG(3, "not_current_day:%d time_value->minute:%d %d\n", not_current_day, time_value->minute, now_time.min);
        if(hour_tai_second) {
            if(now_time.min > time_value->minute) {
                min_tai_second += 60 - now_time.min + time_value->minute;
                *day_tai_flow |= TAI_MINUTE_FLOW; 
            }else {
                min_tai_second += time_value->minute - now_time.min;
            }
            break;
        }
        if(repeat_flag & REPEAT_BY_ONCE_MIN) {
            if(min_rand > now_time.min) {
                min_tai_second += min_rand - now_time.min;
            }else {
                min_tai_second += 60 + min_rand - now_time.min;
                *day_tai_flow |= TAI_MINUTE_FLOW; 
            }
        }else if(repeat_flag & REPEAT_BY_MIN) { 
            if(not_current_day) {
                min_tai_second += 60 - now_time.min;
                *day_tai_flow |= TAI_MINUTE_FLOW; 
                break;
            }
            if(repeat_flag & REPEAT_BY_EVERY20_MIN) {
                min_tai_second += (60 - now_time.min) % 20;
                if(now_time.min > 40)
                    *day_tai_flow |= TAI_MINUTE_FLOW;
            }
            else if(repeat_flag & REPEAT_BY_EVERY15_MIN) {
                min_tai_second += (60 - now_time.min) % 15;
                if(now_time.min > 45)
                    *day_tai_flow |= TAI_MINUTE_FLOW;
            }
            else if(repeat_flag & REPEAT_BY_ANY_MIN){
                min_tai_second += 0;
            }
        }else {
            if(((int)time_value->minute >= now_time.min)) {
                min_tai_second += (int)time_value->minute - now_time.min;
            }else {
                min_tai_second += 60 + (int)time_value->minute - now_time.min;
                *day_tai_flow |= TAI_MINUTE_FLOW; 
            }
        }
    }while(0);

    LOG(3, "minute min_tai_second:%x\n", min_tai_second);

    do{
        int sec_rand = (tick_count % 0x3C);

        if(hour_tai_second || min_tai_second) {
            if(now_time.sec > time_value->second) {
                sec_tai_second += 60 - now_time.sec + time_value->second;
                *day_tai_flow |= TAI_SECOND_FLOW; 
            }else {
                sec_tai_second += time_value->second - now_time.sec;
            }
            break;
        }
        if(repeat_flag & REPEAT_BY_ONCE_SEC) {
            if(sec_rand > now_time.sec) {
                sec_tai_second += sec_rand - now_time.sec;
            }else {
                sec_tai_second += 60 + sec_rand - now_time.sec;
                *day_tai_flow |= TAI_SECOND_FLOW; 
            }
        }else if(repeat_flag & REPEAT_BY_SEC) { 
            if(not_current_day || min_tai_second || hour_tai_second) {
                if(now_time.sec == 0) {
                    ;
                }else {
                    sec_tai_second += 60 - now_time.sec;
                    *day_tai_flow |= TAI_SECOND_FLOW; 
                }
                break;
            }
            if(repeat_flag & REPEAT_BY_EVERY20_SEC) {
                sec_tai_second += (60 - now_time.sec) % 20 ? (60 - now_time.sec) % 20 : 20;
                if(now_time.sec >= 40)
                    *day_tai_flow |= TAI_SECOND_FLOW; 
            }
            else if(repeat_flag & REPEAT_BY_EVERY15_SEC) {
                sec_tai_second += (60 - now_time.sec) % 15 ? (60 - now_time.sec) % 15 : 15;
                if(now_time.sec >= 45)
                    *day_tai_flow |= TAI_SECOND_FLOW; 
            }
            else if(repeat_flag & REPEAT_BY_ANY_SEC){
                sec_tai_second += 1;
                if(now_time.sec == 59)
                    *day_tai_flow |= TAI_SECOND_FLOW; 
            }
        }else {
            if(((int)time_value->second > now_time.sec)) {
                sec_tai_second += (int)time_value->second - now_time.sec;
            }else {
                sec_tai_second += 60 + (int)time_value->second - now_time.sec;
                *day_tai_flow |= TAI_SECOND_FLOW; 
            }
        }
    }while(0);

    LOG(3, "second sec_tai_second:%x\n", sec_tai_second);

    if(*day_tai_flow & TAI_SECOND_FLOW) {
        if(repeat_flag & REPEAT_BY_MIN) {
            if(repeat_flag & REPEAT_BY_EVERY20_MIN) {
                if(now_time.min % 20 == 0)
                    min_tai_second += 20;
                if(now_time.min == 40)
                    *day_tai_flow |= TAI_MINUTE_FLOW;
            }
            else if(repeat_flag & REPEAT_BY_EVERY15_MIN) {
                if(now_time.min % 15 == 0)
                    min_tai_second += 15;
                if(now_time.min == 45)
                    *day_tai_flow |= TAI_MINUTE_FLOW;
            }
            else if(repeat_flag & REPEAT_BY_ANY_MIN){
                if(now_time.min == 59)
                    *day_tai_flow |= TAI_MINUTE_FLOW; 
            }
        }else if(min_tai_second == 0) {
            min_tai_second = 60;
            *day_tai_flow |= TAI_MINUTE_FLOW;
        }
    }
    if(*day_tai_flow & TAI_MINUTE_FLOW) {
        if(repeat_flag & REPEAT_BY_ANY_HOUR){
            if(now_time.hour == 23)
                *day_tai_flow |= TAI_HOUR_FLOW; 
        }else if(hour_tai_second == 0) {
            hour_tai_second = 24;
            *day_tai_flow |= TAI_HOUR_FLOW; 
        }
    }

    if(*day_tai_flow & TAI_SECOND_FLOW) {
        if(min_tai_second)
            min_tai_second -= 1;
        else {
            if(*day_tai_flow & TAI_MINUTE_FLOW)
                min_tai_second = 60 - now_time.min - 1;
        }
    }
    if(*day_tai_flow & TAI_MINUTE_FLOW) {
        if(hour_tai_second)
            hour_tai_second -= 1;
        else {
            if(*day_tai_flow & TAI_HOUR_FLOW)
                hour_tai_second = 24 - now_time.hour - 1;
        }
    }

    LOG(3, "sec_tai_second:%x min_tai_second:%x hour_tai_second:%x\n", sec_tai_second, min_tai_second, hour_tai_second);

    day_tai_second += sec_tai_second + min_tai_second * ONE_MINUTE_TO_S + hour_tai_second * ONE_HOUR_TO_S;

    LOG(3, "day_tai_second:%x day_tai_flow:%x\n", day_tai_second, *day_tai_flow);
    return day_tai_second;
}

uint8_t get_recently_week(scheduler_time_value_t *time_value, tm_date now_time, uint8_t now_week, uint16_t repeat)
{
    uint8_t weekday_min = 0xff;
    for(uint8_t i = 0; i < 7; i++) {
        if(BIT(i) & time_value->dayofweek) {
            if(i == now_week) {
                if(today_time_is_past(time_value, now_time, repeat)) {
                    if(weekday_min >  i + 7)
                        weekday_min = i + 7;
                }else {
                    if(weekday_min > i)
                        weekday_min = i;
                }
            }else if(i < now_week) {
                if(weekday_min >  i + 7)
                    weekday_min = i + 7;
            }else {
                if(weekday_min > i)
                    weekday_min = i;
            }
        }
    }
    LOG(3, "get_recently_week:%d\n", weekday_min);

    return weekday_min;
}

uint8_t get_recently_month(scheduler_time_value_t *time_value, tm_date now_time, uint16_t repeat)
{
    uint8_t month_min = 0xff;
    uint8_t month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint8_t leap_year = ((now_time.year % 4 == 0) && (now_time.year % 100 != 0)) || (now_time.year % 400 == 0); 

    if(leap_year)
        month_day[1] = 29;

    for(uint8_t i = 0, j = i+1; i < 12; i++, j++) {
        if(BIT(i) & time_value->month) {
            if(j > now_time.mon) {
                if(month_min > j)
                    month_min = j;
            }else if(j == now_time.mon) {
                if(now_time.mday > time_value->day) {
                    if(repeat & REPEAT_BY_DAY) {
                        if(now_time.mday == month_day[i]) {
                            if(today_time_is_past(time_value, now_time, repeat)) {
                                if(repeat & REPEAT_BY_MONTH)
                                    if(month_min > j + 12)
                                        month_min = j + 12;
                            }else {
                                if(month_min > j)
                                    month_min = j;
                            }
                        }else {
                            if(month_min > j)
                                month_min = j;
                        }
                    }else {
                        //if((repeat & REPEAT_BY_MONTH) || ((now_time.year % 100) < time_value->year))
                        if(month_min > j + 12)
                            month_min = j + 12;

                    }
                }else if(now_time.mday == time_value->day) {
                    if(today_time_is_past(time_value, now_time, repeat)) {
                        if(repeat & REPEAT_BY_DAY) {
                            if(now_time.mday == month_day[i]) {
                                if(repeat & REPEAT_BY_MONTH)
                                    if(month_min > j + 12)
                                        month_min = j + 12;
                            }
                        }else {
                            if(repeat & REPEAT_BY_MONTH)
                                if(month_min > j + 12)
                                    month_min = j + 12;
                        }

                    }else {
                        if(month_min > j)
                            month_min = j;
                    }
                }else {
                    if(month_min > j)
                        month_min = j;
                }
            }else {
                if(repeat & REPEAT_BY_MONTH)
                    if(month_min > j + 12)
                        month_min = j + 12;
            }
        }
    }
    LOG(3, "get_recently_month:%d\n", month_min);

    return month_min;
}
static int scheduler_get_day_from_year_month_day(int year, int month, int day)
{
    uint8_t leap_year = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0); 
    uint8_t month_day[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int all_day = 0;

    if(leap_year)
        month_day[1] = 29;

    for(int i = 1; i < month; i++) {
        all_day += month_day[i-1];
    }
    all_day += day - 1;

    return all_day;
}
static int scheduler_get_leap_times_from_year_interval(int year_small, int year_big)
{
    int leap_count = 0;
    for(int i = year_small + 1; i < year_big; i++) {
         if((i % 400 == 0 ) || (i % 4 == 0 && i % 100 != 0))
        {
            leap_count++;
        }
    }
    return leap_count;
}

static uint64_t scheduler_caculate_timer_time(scheduler_server_t *server)
{
    uint16_t repeat_flag = server->scheduler_time->repeat_flag;  
    tm_date now_time;  /* get now_time */
    uint64_t tai_second = 0;
    scheduler_time_value_t *time_value = &server->scheduler_time->time_value;
    uint8_t leap_year = 0;
    uint8_t not_current_day = 0;

    get_system_time(&now_time);

    leap_year = ((now_time.year % 4 == 0) && (now_time.year % 100 != 0)) || (now_time.year % 400 == 0); 

    LOG(3, "scheduler_caculate_timer_time repeat:%x\n", repeat_flag);

    server->scheduler_time->day_tai_flow = 0;

    if( repeat_flag & REPEAT_BY_WEEK) {
        uint8_t now_week = getWeekdayByYearday(now_time.year, now_time.mon, now_time.mday);
        uint8_t day_week = get_recently_week(time_value, now_time, now_week, repeat_flag);
        not_current_day = (time_value->year != (now_time.year % 100) && !(repeat_flag & REPEAT_BY_YEAR)) || (now_week != day_week);
        tai_second += get_day_min_tai_second(time_value, now_time, repeat_flag, &server->scheduler_time->day_tai_flow, not_current_day);
        LOG(3, "scheduler_caculate_timer_time week:%d %d day_tai_weei:%x\n", now_week, day_week, server->scheduler_time->day_tai_flow);
        if(server->scheduler_time->day_tai_flow & TAI_HOUR_FLOW)
            tai_second += (day_week - now_week - 1) * ONE_DAY_TO_S;
        else
            tai_second += (day_week - now_week) * ONE_DAY_TO_S;
    }else {
        uint8_t recent_month = get_recently_month(time_value, now_time, repeat_flag) ; /* get recently month */
        uint32_t now_day = scheduler_get_day_from_year_month_day(now_time.year, now_time.mon, now_time.mday);
        uint32_t month_day;
        uint16_t year = now_time.year+1;
        uint8_t next_year_leap = ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0); 
        uint8_t time_day = 0;
        if((time_value->year != (now_time.year % 100) && !(repeat_flag & REPEAT_BY_YEAR)) || (recent_month > now_time.mon) || (time_value->day > now_time.mday))
            not_current_day = 1;

        LOG(3, "now_time.mday:%d time_value->day:%d %d\n", now_time.mday, time_value->day, today_time_is_past(time_value, now_time, repeat_flag));
        if(repeat_flag & REPEAT_BY_DAY) {
            if((time_value->year != (now_time.year % 100) && !(repeat_flag & REPEAT_BY_YEAR)) || recent_month > now_time.mon) {
                time_day = 1;
                not_current_day = 1;
            }
            else if(today_time_is_past(time_value, now_time, repeat_flag)) {
                time_day = now_time.mday + 1;
                not_current_day = 1;
            }else
                time_day = now_time.mday;
        }else {
            time_day = time_value->day;
        }

        LOG(3, "year:%d recent_month:%d leap_year:%d, next_year_leap:%d time_day:%d now_day:%d\n", 
                time_value->year > (now_time.year % 100), recent_month, leap_year, next_year_leap, time_day, now_day);
        if(recent_month > 12) {

            month_day = 365 + leap_year - now_day + scheduler_get_day_from_year_month_day(now_time.year + 1, recent_month - 12, time_day);
            LOG(3, "month_day:%d\n", month_day);

            server->scheduler_time->day_tai_flow |= TAI_MONTH_FLOW;
        }else {
            month_day = scheduler_get_day_from_year_month_day(now_time.year, recent_month, time_day) - now_day; 

            if((time_value->year != (now_time.year % 100) && !(repeat_flag & REPEAT_BY_YEAR)) && (recent_month > 2) && next_year_leap)
                month_day += 1;
        }

        tai_second += get_day_min_tai_second(time_value, now_time, repeat_flag, &server->scheduler_time->day_tai_flow, not_current_day);

        if(server->scheduler_time->day_tai_flow & TAI_HOUR_FLOW)
        {
            month_day -= 1;
        }
        LOG(3, "scheduler_caculate_timer_time recent_month:%d now_day:%d month_day:%d day_tai_flow:%x leap_year:%d\n",
                recent_month, now_day, month_day, server->scheduler_time->day_tai_flow, leap_year);

        tai_second += month_day * ONE_DAY_TO_S;
    }

    LOG(3, "scheduler_caculate_timer_time tai_second:%x\n", tai_second);
    if(repeat_flag & REPEAT_BY_YEAR) {
        tai_second += 0;
    }else {
        int month_flow = 0;
        if(server->scheduler_time->day_tai_flow & TAI_MONTH_FLOW) {
            month_flow = 1;
        }
        if(time_value->year >= (now_time.year % 100)) {
            tai_second += (time_value->year - (now_time.year % 0x64) - month_flow) * ONE_YEAR_TO_S;
            tai_second += scheduler_get_leap_times_from_year_interval(now_time.year, (now_time.year / 100) * 100 
                             + time_value->year) * ONE_DAY_TO_S;
        } else {
            tai_second += (100 + time_value->year - (now_time.year % 0x64) - month_flow) * ONE_YEAR_TO_S;
            tai_second += scheduler_get_leap_times_from_year_interval(
                    now_time.year, (now_time.year / 100) * 100 + time_value->year + 100) * ONE_DAY_TO_S;
        }
    }

    return tai_second;
}


static int8_t scheduler_get_least_alarm_time(scheduler_server_t *server, int current_index)
{
    int8_t alarm_min = 0;
    uint32_t expiry = 0;

    if(server->scheduler_time->alarm_num >= 0) {
        expiry = TICKS_TO_pdS(mesh_timer_get_remain_time(server->scheduler_time->scheduler_Timer));
        scheduler_minus_elapsed_time(server, server->scheduler_time->alarm_time[server->scheduler_time->alarm_num] - server->scheduler_time->alarm_remain_time - expiry, current_index);
    }

    LOG(3, "alarm_time[0]:%x alarm_time[%d]:%x\n", 
            (uint32_t)(server->scheduler_time->alarm_time[0] & 0xffffffff), server->scheduler_time->alarm_num, (uint32_t)(server->scheduler_time->alarm_time[server->scheduler_time->alarm_num] & 0xffffffff));
    for(int i = 1; i < SCHSDULER_INDEX_MAX; i++) {
        //LOG(3, "alarm_time[%d]:%x\n", i, (uint32_t)(server->scheduler_time->alarm_time[i] & 0xffffffff));
        if(server->scheduler_time->alarm_time[i]
                && server->scheduler_time->alarm_time[i] < server->scheduler_time->alarm_time[alarm_min])
            alarm_min = i;
    }

    return alarm_min;
}

static void scheduler_time_handler(scheduler_server_t *server)
{
    uint32_t tick_count = xTaskGetTickCount();
    uint64_t tai_second = 0;
    int8_t alarm_min = 0;
    uint32_t delay_time = 0;
    int current_index;

    memset(&server->scheduler_time->time_value, 0, sizeof(scheduler_time_value_t));
    server->scheduler_time->repeat_flag = 0;

    if(server->msg_format->action_set.year < 0x64)
        server->scheduler_time->time_value.year = server->msg_format->action_set.year % 100;
    else {
        server->scheduler_time->time_value.year = server->msg_format->action_set.year;
        server->scheduler_time->repeat_flag |= REPEAT_BY_YEAR;
    }

    if(server->msg_format->action_set.month) {
        server->scheduler_time->time_value.month = server->msg_format->action_set.month;
        server->scheduler_time->time_value.day = server->msg_format->action_set.day;

        if(server->scheduler_time->repeat_flag & REPEAT_BY_YEAR)
            server->scheduler_time->repeat_flag |= REPEAT_BY_MONTH;

        if(server->msg_format->action_set.day == 0) {
            server->scheduler_time->time_value.day = 0;
            server->scheduler_time->repeat_flag |= REPEAT_BY_DAY;
        }
    }else if(server->msg_format->action_set.dayofweek) {
        server->scheduler_time->time_value.dayofweek = server->msg_format->action_set.dayofweek;
        server->scheduler_time->time_value.day = 0;
        server->scheduler_time->repeat_flag |= REPEAT_BY_WEEK;
    }

    if(server->msg_format->action_set.hour < 0x18) {
        server->scheduler_time->time_value.hour = server->msg_format->action_set.hour;
    }else if(server->msg_format->action_set.hour == 0x18) {
        server->scheduler_time->time_value.hour = 0;
        server->scheduler_time->repeat_flag |= REPEAT_BY_ANY_HOUR;
    }else if(server->msg_format->action_set.hour == 0x19) {
        server->scheduler_time->time_value.hour = tick_count % 0x18;
        server->scheduler_time->repeat_flag |= REPEAT_BY_ONCE_HOUR;
    }

    if(server->msg_format->action_set.minute < 0x3C) {
        server->scheduler_time->time_value.minute = server->msg_format->action_set.minute;
    }else if(server->msg_format->action_set.minute == 0x3C) {
        server->scheduler_time->time_value.minute = 0;
        server->scheduler_time->repeat_flag |= REPEAT_BY_ANY_MIN;
    }else if(server->msg_format->action_set.minute == 0x3D) {
        server->scheduler_time->time_value.minute = 0;
        server->scheduler_time->repeat_flag |= REPEAT_BY_EVERY15_MIN;
    }else if(server->msg_format->action_set.minute == 0x3E) {
        server->scheduler_time->time_value.minute = 0;
        server->scheduler_time->repeat_flag |= REPEAT_BY_EVERY20_MIN;
    }else if(server->msg_format->action_set.minute == 0x3F) {
        server->scheduler_time->time_value.minute = tick_count % 0x3B;
        server->scheduler_time->repeat_flag |= REPEAT_BY_ONCE_MIN;
    }

    if(server->msg_format->action_set.second < 0x3C) {
        server->scheduler_time->time_value.second = server->msg_format->action_set.second;
    }else if(server->msg_format->action_set.second == 0x3C) {
        server->scheduler_time->time_value.second = 0;
        server->scheduler_time->repeat_flag |= REPEAT_BY_ANY_SEC;
    }else if(server->msg_format->action_set.second == 0x3D) {
        server->scheduler_time->time_value.second = 0;
        server->scheduler_time->repeat_flag |= REPEAT_BY_EVERY15_SEC;
    }else if(server->msg_format->action_set.second == 0x3E) {
        server->scheduler_time->time_value.second = 0;
        server->scheduler_time->repeat_flag |= REPEAT_BY_EVERY20_SEC;
    }else if(server->msg_format->action_set.second == 0x3F) {
        server->scheduler_time->time_value.second = tick_count % 0x3B;
        server->scheduler_time->repeat_flag |= REPEAT_BY_ONCE_SEC;
    }

    server->scheduler_time->time_value.action = server->msg_format->action_set.action;
#if 0
    LOG(3, "msg_format year:%d\n", server->msg_format->action_set.year);
    LOG(3, "month:%d\n", server->msg_format->action_set.month);
    LOG(3, "day:%d\n", server->msg_format->action_set.day);
    LOG(3, "hour:%d\n", server->msg_format->action_set.hour);
    LOG(3, "minute:%d\n", server->msg_format->action_set.minute);
    LOG(3, "second:%d\n", server->msg_format->action_set.second);
    LOG(3, "dayofweek:%d\n", server->msg_format->action_set.dayofweek);
    LOG(3, "action:%d\n", server->msg_format->action_set.action);
#endif
#if 0
    LOG(3, "scheduler_time year:%d\n", server->scheduler_time->time_value.year);
    LOG(3, "month:%d\n", server->scheduler_time->time_value.month);
    LOG(3, "day:%d\n", server->scheduler_time->time_value.day);
    LOG(3, "hour:%d\n", server->scheduler_time->time_value.hour);
    LOG(3, "minute:%d\n", server->scheduler_time->time_value.minute);
    LOG(3, "second:%d\n", server->scheduler_time->time_value.second);
    LOG(3, "dayofweek:%d\n", server->scheduler_time->time_value.dayofweek);
    LOG(3, "action:%d\n", server->scheduler_time->time_value.action);
    LOG(3, "repeat_flag:%x\n", server->scheduler_time->repeat_flag);
#endif
#if 1
    LOG(3, "scheduler_time_handler alarm_num:%x\n", server->scheduler_time->alarm_num);
    tai_second = scheduler_caculate_timer_time(server);

    LOG(3, "scheduler_time_handler tai_second:%x\n", tai_second);


    for(int i = 0; i < SCHSDULER_INDEX_MAX; i++) {
        if(server->scheduler_time->alarm_time[i] == 0) {
            server->scheduler_time->alarm_time[i] = tai_second;
            server->scheduler_time->alarm_timer_index[i] = server->msg_format->action_set.index;
            current_index = i;
            LOG(3, "current_index:%d, alarm_timer_index[%d]:%x\n", current_index, i, server->scheduler_time->alarm_timer_index[i]);
            break;
        }
    }
#if 0
    for(int i = 0; i < SCHSDULER_INDEX_MAX; i++) {
        LOG(3, "1scheduler_time_handler alarm_time[%d]:%x\n", i, (uint32_t)(server->scheduler_time->alarm_time[i] & 0xffffffff));
    }
#endif

    alarm_min = scheduler_get_least_alarm_time(server, current_index);
    for(int i = 0; i < SCHSDULER_INDEX_MAX; i++) {
        LOG(3, "scheduler_time_handler alarm_time[%d]:%x\n", i, (uint32_t)(server->scheduler_time->alarm_time[i] & 0xffffffff));
    }
    LOG(3, "last_timeout_flag:%d alarm_num:%d alarm_min:%d\n", 
            server->scheduler_time->last_timeout_flag, server->scheduler_time->alarm_num, alarm_min);
    //if(server->scheduler_time->last_timeout_flag || server->scheduler_time->alarm_num == alarm_min) {/* for set same time with running time bug */
    if(server->scheduler_time->last_timeout_flag) {
        return ;
    }

    if(server->scheduler_time->scheduler_Timer)
    {
        mesh_timer_delete(server->scheduler_time->scheduler_Timer);
        server->scheduler_time->scheduler_Timer = NULL;
    }

    server->scheduler_time->alarm_num = alarm_min;

    if(server->scheduler_time->alarm_time[server->scheduler_time->alarm_num] > TIMER_MAX_DELAY_SEC) {
        delay_time = TIMER_MAX_DELAY_SEC;
        server->scheduler_time->alarm_remain_time = server->scheduler_time->alarm_time[server->scheduler_time->alarm_num] - TIMER_MAX_DELAY_SEC;
    }else {
        delay_time = (uint32_t)server->scheduler_time->alarm_time[server->scheduler_time->alarm_num];
        server->scheduler_time->alarm_remain_time = 0;
    }
    LOG(3, "alarm_time:%x\n", server->scheduler_time->alarm_time[server->scheduler_time->alarm_num] & 0xffffffff);
    LOG(3, "remain_time:%x\n", server->scheduler_time->alarm_remain_time & 0xffffffff);
    LOG(3, "delay_time:%x %x\n", delay_time, pdS_TO_TICKS(delay_time));
    server->scheduler_time->alarm_count = 0;

    for(int i = 0, j = 0; i < SCHSDULER_INDEX_MAX; i++) {
        //LOG(3, "alarm_timer_index[%d]:%x\n", i, server->scheduler_time->alarm_timer_index[i]);
        if(server->scheduler_time->alarm_time[server->scheduler_time->alarm_num] == server->scheduler_time->alarm_time[i]) {
            server->scheduler_time->current_alarm_index[j] = server->scheduler_time->alarm_timer_index[i]; 
            server->scheduler_time->alarm_count++;
            LOG(3, "current_alarm_index[%d]:%x\n", j, server->scheduler_time->current_alarm_index[j]);
            j++;
        }
    }

    if(server->scheduler_time->scheduler_Timer == NULL) {
        if(delay_time == 0)
            delay_time = 1;
        server->scheduler_time->scheduler_Timer = mesh_timer_create(
                "scheduler", pdS_TO_TICKS(delay_time), pdFALSE, (void *)server, scheduler_server_action_timer_handle);

        if(server->scheduler_time->scheduler_Timer != NULL)
            mesh_timer_start(server->scheduler_time->scheduler_Timer);
    }
#endif
}
static void scheduler_action_delete_by_index(scheduler_server_t *server, uint8_t index)
{
    int alarm_index = -1;
    int alarm_count = server->scheduler_time->alarm_count;
    int current_alarm_flag = 0;
    int8_t other_alarm_index = 0;
    for(int i = 0; i < SCHSDULER_INDEX_MAX; i++) {
        if(server->scheduler_time->alarm_timer_index[i] == index) {
            alarm_index = i;
            break;
        }
    }

    LOG(3, "scheduler_action_delete_by_index index:%d alarm_index:%d\n", index, alarm_index);
    server->scheduler_time->alarm_time[alarm_index] = 0;

    for(int i = 0; i < SCHSDULER_INDEX_MAX; i++) {
        if(server->scheduler_time->current_alarm_index[i] >= 0) {
            if(server->scheduler_time->current_alarm_index[i] != index)
                other_alarm_index = server->scheduler_time->current_alarm_index[i];

            if(server->scheduler_time->current_alarm_index[i] == index) {
                server->scheduler_time->current_alarm_index[i] = -1;
                current_alarm_flag = 1;
            }
        }

    }
    LOG(3,"alarm_count:%d current_alarm_flag:%d alarm_num:%d\n", alarm_count, current_alarm_flag, server->scheduler_time->alarm_num);

    if(current_alarm_flag) {
        if(alarm_count == 1) {
            server->scheduler_time->alarm_remain_time = 0;
            server->scheduler_time->alarm_num = -1;
            mesh_timer_delete(server->scheduler_time->scheduler_Timer);
            server->scheduler_time->scheduler_Timer = NULL;
        }else {
            if(server->scheduler_time->alarm_num == alarm_index) {
                 for(int i = 0; i < SCHSDULER_INDEX_MAX; i++) {
                     if(server->scheduler_time->alarm_timer_index[i] == other_alarm_index) {
                         server->scheduler_time->alarm_num = i;
                         break;
                     }
                 }
            }
            for(int i = 0; i < alarm_count - 1; i++) {
                if(server->scheduler_time->current_alarm_index[i] >= 0)
                    continue;
                else {
                    for(int j = i + 1; j < SCHSDULER_INDEX_MAX; j++) {
                        if(server->scheduler_time->current_alarm_index[j] >= 0) {
                            server->scheduler_time->current_alarm_index[i] = server->scheduler_time->current_alarm_index[j];
                            server->scheduler_time->current_alarm_index[j] = -1;
                        }
                    }
                }
            }
        }

        server->scheduler_time->alarm_count--;
    }
    server->scheduler_time->alarm_timer_index[alarm_index] = -1;
}

static int scheduler_action_set_rx_handler(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t repeat_flag = 0;
    scheduler_server_t *server = GET_SERVER_MODEL_PTR(scheduler_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);
    scheduler_action_set_t *p_pdu = (scheduler_action_set_t *)(access + 1);
    if(p_pdu->year > 0x64 || p_pdu->hour > 0x19)
        return -1;
    
    memcpy(&server->msg_format->action_set, p_pdu, sizeof(scheduler_action_set_t));
#if 0
    server->msg_format->action_set.index = p_pdu->index;
    server->msg_format->action_set.year = p_pdu->year;
    server->msg_format->action_set.month = p_pdu->month;
    server->msg_format->action_set.day = p_pdu->day;
    server->msg_format->action_set.hour = p_pdu->hour;
    server->msg_format->action_set.minute = p_pdu->minute;
    server->msg_format->action_set.second = p_pdu->second;
    server->msg_format->action_set.dayofweek = p_pdu->dayofweek;
    server->msg_format->action_set.action = p_pdu->action;
    server->msg_format->action_set.trans_time = p_pdu->trans_time;
    server->msg_format->action_set.scene_number = p_pdu->scene_number;
#endif

    LOG(3, "scheduler_action_set_rx_handler:%x %x minute:%x\n", server->msg_format->schedules, 1 << server->msg_format->action_set.index, server->msg_format->action_set.minute);
    if(server->msg_format->schedules & (1 << server->msg_format->action_set.index)) {
        scheduler_action_delete_by_index(server, server->msg_format->action_set.index);
        repeat_flag = 1;
    }else {
        server->msg_format->schedules |= 1 << server->msg_format->action_set.index;
    }


    scheduler_call_user_callback(server, TIME_SCHEDULER_EVT_ACTION_SET, (uint16_t)p_pdu->index, repeat_flag);
    scheduler_time_handler(server);
    LOG(3, "scheduler_action_set_rx_handler schedules:%x\n", server->msg_format->schedules);

    return 0;
}

void scheduler_action_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    if(scheduler_action_set_rx_handler(elmt, model, pdu) == 0)
        send_scheduler_action_status(elmt, model, pdu);
}

void scheduler_action_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scheduler_action_set_rx_handler(elmt, model, pdu);
}

static int scheduler_action_get_rx_handler(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scheduler_server_t *server = GET_SERVER_MODEL_PTR(scheduler_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);
    scheduler_action_get_t *p_pdu = (scheduler_action_get_t *)(access + 2);
    LOG(3, "scheduler_action_get_rx_handler index:%x\n", p_pdu->index);
    if(p_pdu->index <= 0x0f) {
        scheduler_call_user_callback(server, TIME_SCHEDULER_EVT_ACTION_GET, (uint16_t)p_pdu->index, 0);
        return 0;
    }

    return -1;
}

void scheduler_action_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    if(scheduler_action_get_rx_handler(elmt, model, pdu) == 0) {
        LOG(3, "scheduler_action_get_rx index:%x\n", *(uint8_t *)(access_get_pdu_payload(pdu) + 2));
        send_scheduler_action_status(elmt, model, pdu);
    }
}

static void scheduler_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "%s\n", __func__);
}
static void send_scheduler_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    scheduler_server_t *server = GET_SERVER_MODEL_PTR(scheduler_server_t, model);
    scheduler_status_t msg;
    msg.schedules = server->msg_format->schedules;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(TIME_AND_SCENE_ONE_OCTET_OPCODE_OFFSET, Scheduler_Action_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(scheduler_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,scheduler_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}

void scheduler_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_scheduler_status(elmt, model, pdu);
}
