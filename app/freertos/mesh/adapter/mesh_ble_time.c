#include "mesh_ble_time.h"
#include "os_wrapper.h"
#include "bx_dbg.h"
#include "FreeRTOS.h"
#include "co_bt_defines.h"
#include "bx_config.h"
#include "bx_ring_queue.h"
#include "task_init.h"
#include "rtc.h"
#include "rc_calib.h"

#define ADV_RX_TIME_QUEUE_LENGTH 20
DEF_RING_QUEUE(adv_rx_time_q,ADV_RX_TIME_QUEUE_LENGTH,ble_txrx_time_t);

void adv_rx_time_enqueue(uint32_t rtc_time)
{
    ble_txrx_time_t time = {
        .time_cnt = rtc_time,
    };
    BX_ASSERT(bx_ring_queue_full(&adv_rx_time_q)==false);
    bx_enqueue(&adv_rx_time_q,&time);
}

ble_txrx_time_t *adv_rx_time_dequeue()
{
    ble_txrx_time_t *time;
    os_enter_critical();
    BX_ASSERT(bx_ring_queue_empty(&adv_rx_time_q)==false);
    time = bx_dequeue(&adv_rx_time_q);
    os_exit_critical();
    return time;
}

ble_txrx_time_t ble_current_time_get()
{
    ble_txrx_time_t time;
    os_enter_critical();
    time.time_cnt = RTC_CURRENTCNT_GET(); 
    os_exit_critical();
    return time;
}

uint32_t ble_time_to_os_tick(ble_txrx_time_t ble_time)
{
    return configTICK_RATE_HZ*ble_time.time_cnt/portRTC_FREQUENCY;
}

ble_txrx_time_t os_tick_to_ble_time(uint32_t ticks)
{
    ble_txrx_time_t time;
    time.time_cnt = portRTC_FREQUENCY*ticks/configTICK_RATE_HZ;
    return time;
}

ble_txrx_time_t mesh_ms_to_ble_time(uint32_t ms)
{
    ble_txrx_time_t time;
    time.time_cnt = portRTC_FREQUENCY*ms/1000;
    return time;
}

ble_txrx_time_t mesh_time_add(ble_txrx_time_t a,ble_txrx_time_t b)
{
    ble_txrx_time_t sum;
    sum.time_cnt = a.time_cnt + b.time_cnt;
    return sum;
}

int32_t mesh_time_diff(ble_txrx_time_t a,ble_txrx_time_t b)
{
    return time_diff(a.time_cnt,b.time_cnt);
}
